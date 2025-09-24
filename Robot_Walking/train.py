"""Sample script for training a control policy on the Hopper environment

    Read the stable-baselines3 documentation and implement a training
    pipeline with an RL algorithm of your choice between TRPO, PPO, and SAC.
"""
import gym
import matplotlib.pyplot as plt
import argparse


import os
os.environ["KMP_DUPLICATE_LIB_OK"]="TRUE"

from env.custom_hopper import *

from stable_baselines3 import PPO, SAC
from stable_baselines3.common.evaluation import evaluate_policy

from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.results_plotter import load_results, ts2xy

def create_model(args, env):
    # T4 implement ppo algorithm
    if args.algo == 'ppo':
        model = PPO("MlpPolicy", env, learning_rate=args.lr, verbose=1)

    elif args.algo == 'sac':
        model = SAC("MlpPolicy", env, learning_rate=args.lr, verbose=1)
    else:
        raise ValueError(f"RL Algo not supported: {args.algo}")
    return model

def load_model(args, env):
    if args.algo == 'ppo':
        model = PPO.load(args.test, env=env)
    elif args.algo == 'sac':
        model = SAC.load(args.test, env=env)
    else:
        raise ValueError(f"RL Algo not supported: {args.algo}")
    return model

def moving_average(values, window):
    """
    Smooth values by doing a moving average
    :param values: (numpy array)
    :param window: (int)
    :return: (numpy array)
    """
    weights = np.repeat(1.0, window) / window
    return np.convolve(values, weights, "valid")


def plot_results(log_folder, title="Learning Curve"):
    """
    plot the results

    :param log_folder: (str) the save location of the results to plot
    :param title: (str) the title of the task to plot
    """
    x, y = ts2xy(load_results(log_folder), "timesteps")
    y = moving_average(y, window=50)
    # Truncate x
    x = x[len(x) - len(y) :]

    fig = plt.figure(title)
    plt.plot(x, y)
    plt.xlabel("Number of Timesteps")
    plt.ylabel("Rewards")
    plt.title(title + " Smoothed")
    plt.show()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--udr_massa", action='store_true', help="Use udr")
    parser.add_argument("--udr_mappa", action='store_true', help="Use udr")
    parser.add_argument("--test", "-t", type=str, default=None, help="Model to be tested")
    parser.add_argument("--render_test", action='store_true', help="Render test")
    parser.add_argument('--algo', default='ppo', type=str, help='RL Algo [ppo, sac]')
    parser.add_argument('--lr', default=0.001, type=float, help='Learning rate')
    parser.add_argument("--total_timesteps", type=int, default=400000, help="The total number of samples to train on")
    parser.add_argument('--test_episodes', default=50, type=int, help='# episodes for test evaluations')
    args = parser.parse_args()

    log_dir = "./tmp/gym/"
    os.makedirs(log_dir, exist_ok=True)

    print("UDR Massa: " + str(args.udr_massa))
    print("UDR Mappa: " + str(args.udr_mappa))

    filename = f"{args.algo}_hopper"
    if (args.udr_massa == True):
        filename += "_udrMassa"
    else:
        filename += "_NOudrMassa"

    if (args.udr_mappa == True):
        filename += "_udrMappa"
    else:
        filename += "_NOudrMappa"

    # If no model was passed, train a policy from scratch.
    # Otherwise load the model from the file and go directly to testing.
    if args.test is None:
        try:
            env = gym.make('CustomHopper-source-v0', udr_massa=args.udr_massa, udr_mappa=args.udr_mappa)
            env = Monitor(env, log_dir)
            print('State space:', env.observation_space)
            print('Action space:', env.action_space)
            print('Dynamics parameters:', env.get_parameters())

            model = create_model(args, env)
            model.learn(total_timesteps=args.total_timesteps)
            model.save(filename)
            plot_results(log_dir)
            # Handle Ctrl+C - save model and go to tests
        except KeyboardInterrupt:
            print("Interrupted!")
    else:
        # Test the model on SOURCE environment
        print("Testing...")

        # SOURCE SOURCE
        # env = gym.make('CustomHopper-source-v0', udr_massa=False, udr_mappa=False)
        # env = Monitor(env, log_dir)
        # print('State space:', env.observation_space)
        # print('Action space:', env.action_space)
        # print('Dynamics parameters:', env.get_parameters())
        # model = load_model(args, env)
        # mean_reward, std_reward = evaluate_policy(model, env, n_eval_episodes=args.test_episodes, render=args.render_test)
        
        # print(f"Test reward (avg +/- std): ({mean_reward} +/- {std_reward}) - Num episodes: {args.test_episodes}")

        # SOURCE TARGET
        env = gym.make('CustomHopper-target-v0', udr_massa=False, udr_mappa=False)
        env = Monitor(env, log_dir)
        print('State space:', env.observation_space)
        print('Action space:', env.action_space)
        print('Dynamics parameters:', env.get_parameters())
        model = load_model(args, env)
        mean_reward, std_reward = evaluate_policy(model, env, n_eval_episodes=args.test_episodes, render=args.render_test)

        print(f"Test reward (avg +/- std): ({mean_reward} +/- {std_reward}) - Num episodes: {args.test_episodes}")

    env.close()

if __name__ == '__main__':
    main()