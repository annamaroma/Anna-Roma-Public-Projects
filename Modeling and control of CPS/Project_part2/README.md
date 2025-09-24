# Modeling Project Part II
## File list
- main.m File for single simulation
- SVFB.slx Simulink main file for simulation with observer
- SVFB_localObserver.slx Simulink main file for simulation with localObserver
- closed_loop_dynamics.m Function called in SVFB.slx and SVFB_localObserver.slx, is the dynamic of a single node
- closed_loop_observer.m Function called in SVFB.slx, is the observer of a single node
- closed_loop_local_observer.m Function called in SVFB_localObserver.slx, is the observer of a single node
- Node_wObserver.slx Simulink block of a single node with observer
- Node_wLocalObserver.slx Simulink block of a single node with localObserver
- cluster/* Matlab files to run multiple simulations in one go, with saved data for plots
- notclean/* First versions, not cleaned, of the simulations (used for debug)