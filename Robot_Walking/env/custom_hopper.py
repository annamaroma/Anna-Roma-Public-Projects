"""Implementation of the Hopper environment supporting
domain randomization optimization."""
import csv
import pdb
from copy import deepcopy

import numpy as np
import gym
from gym import utils
from .mujoco_env import MujocoEnv
from scipy.stats import truncnorm
from scipy.spatial.transform import Rotation

class CustomHopper(MujocoEnv, utils.EzPickle):
    def __init__(self, domain=None, udr_massa=False, udr_mappa=False):
        MujocoEnv.__init__(self, 4)
        utils.EzPickle.__init__(self)

        self.original_masses = np.copy(self.sim.model.body_mass[1:])    # Default link masses
        self.udr_massa = udr_massa
        self.udr_mappa = udr_mappa
        self.domain = domain

        if domain == 'source':  # Source environment has an imprecise torso mass (1kg shift)
            self.sim.model.body_mass[2] -= 1.0



    def set_random_parameters(self):
        """Set random masses
        TODO
        """
        self.set_parameters(self.sample_parameters())

    def sample_parameters(self):
        """Sample masses according to a domain randomization distribution
        TODO
        """
        
        sample_masses=[]

        sample_masses.append(0) # Added for terrain
        sample_masses.append(self.sim.model.body_mass[2])
        sample_masses.append(np.random.uniform(2.92699082, 4.92699082))
        sample_masses.append(np.random.uniform(1.71433605, 3.71433605))
        sample_masses.append(np.random.uniform(4.0893801, 6.0893801))


        return np.array(sample_masses)

    def get_parameters(self):
        """Get value of mass for each link"""
        masses = np.array( self.sim.model.body_mass[1:] )
        return masses
    
    def get_body_names(self):
        """Get name for each link"""
        masses = np.array( self.sim.model.body_names[1:] )
        return masses

    def set_parameters(self, task):
        """Set each hopper link's mass to a new value"""
        self.sim.model.body_mass[1:] = task

    def step(self, a):
        """Step the simulation to the next timestep

        Parameters
        ----------
        a : ndarray,
            action to be taken at the current timestep
        """
        posbefore = self.sim.data.qpos[0]
        self.do_simulation(a, self.frame_skip)
        posafter, height, ang = self.sim.data.qpos[0:3]
        alive_bonus = 1.0
        reward = ((posafter - posbefore) / self.dt - 0.05)
        reward += alive_bonus
        reward -= 1e-3 * np.square(a).sum()
        s = self.state_vector()
        done = not (np.isfinite(s).all() and (np.abs(s[2:]) < 100).all() and (height > .7) and (abs(ang) < .2))
        ob = self._get_obs()

        return ob, reward, done, {}

    def _get_obs(self):
        """Get current state"""
        return np.concatenate([
            self.sim.data.qpos.flat[1:],
            self.sim.data.qvel.flat
        ])

    def reset_model(self):
        """Reset the environment to a random initial state"""
        if self.udr_massa:
            self.set_random_parameters()

        if self.domain == "target":
            self.model.body_quat[1] = Rotation.from_euler('xyz', [180, 0, 0], degrees=True).as_quat()
        else:
            if self.udr_mappa:
                rotation_angle = self.np_random.uniform(low=175, high=185)
                self.model.body_quat[1] = Rotation.from_euler('xyz', [rotation_angle, 0, 0], degrees=True).as_quat()
            else:
                self.model.body_quat[1] = Rotation.from_euler('xyz', [183, 0, 0], degrees=True).as_quat()
            
        qpos = self.init_qpos + self.np_random.uniform(low=-.005, high=.005, size=self.model.nq)
        qvel = self.init_qvel + self.np_random.uniform(low=-.005, high=.005, size=self.model.nv)
        self.set_state(qpos, qvel)

        #print("------------",  self.sim.model.body_mass[2])

        return self._get_obs()

    def viewer_setup(self):
        self.viewer.cam.trackbodyid = 2
        self.viewer.cam.distance = self.model.stat.extent * 0.75
        self.viewer.cam.lookat[2] = 1.15
        self.viewer.cam.elevation = -20



"""
    Registered environments
"""
gym.envs.register(
        id="CustomHopper-v0",
        entry_point="%s:CustomHopper" % __name__,
        max_episode_steps=500,
)

gym.envs.register(
        id="CustomHopper-source-v0",
        entry_point="%s:CustomHopper" % __name__,
        max_episode_steps=500,
        kwargs={"domain": "source"}
)

gym.envs.register(
        id="CustomHopper-target-v0",
        entry_point="%s:CustomHopper" % __name__,
        max_episode_steps=500,
        kwargs={"domain": "target"}
)

