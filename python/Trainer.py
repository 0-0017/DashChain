# Trainer.py

import os
import csv
import optuna
import numpy as np
from RL import DashEnv
from stable_baselines3 import SAC
from stable_baselines3.common.monitor import Monitor
from stable_baselines3.common.env_checker import check_env
from stable_baselines3.common.evaluation import evaluate_policy

def load_data(env_data_dict):
    """
    Accepts a dictionary of environment values.
    loads to cache
    """

    # save data to cache
    file_path = 'data_cache.csv'
    file_exists = os.path.exists(file_path)
    write_header = not file_exists or os.path.getsize(file_path) == 0
    with open(file_path, 'a', newline='') as f:
        writer = csv.writer(f)
        if write_header:
            writer.writerow(env_data_dict.keys())  # ✅ Write header only once
        writer.writerow(env_data_dict.values())

        # read length of file
    with open('data_cache.csv', 'r') as f:
        reader = csv.reader(f)
        row_count = sum(1 for row in reader) - 1  # subtract 1 for header
        print(f"Row count: {row_count}")

    # training & retraining
    train_count = 0
    if row_count >= 32:
        prediction = train_model(env_data_dict)
        open('data_cache.csv', 'w').close()
        return {"prediction": prediction}
    else:
        return {"prediction":[]}


def train_model(env_data_dict):
    """
    Accepts a dictionary of initial environment values.
    Trains the SAC model and returns predicted actions for 100 steps.
    """

    env = DashEnv()
    monitored_env = Monitor(env)
    check_env(monitored_env, warn=True)

    np_data = np.genfromtxt('data_cache.csv', delimiter=',', skip_header=1)
    np_data = np_data[~np.isnan(np_data).any(axis=1)]  # Remove rows with NaNs

    # Train model
    model = SAC("MlpPolicy", monitored_env, verbose=0)
    for row in np_data:
        # Expecting row = [total_supply, circ_supply, balance, votes, height, tx_volume]
        env.total_supply = row[0]
        env.circulating_supply = row[1]
        env.delegate_balance = row[2]
        env.votes_per_period = row[3]
        env.block_height = row[4]
        env.tx_volume = row[5]

        model.learn(total_timesteps=200)  # Adjust as needed 120_000

    model.save("sac_dash_model")

    # Hyper Parameter Tuning
    best_params = hyper_tuning()
    model = SAC("MlpPolicy", env, **best_params)  # ← inject the tuned parameters
    model.learn(total_timesteps=150) # 120000
    model.save("sac_dash_model_optimized")

    # Evaluate SAC model on DashEnv
    mean_reward, std_reward = evaluate_policy(model, env, n_eval_episodes=15, deterministic=True) # 300
    print(f"Mean reward: {mean_reward}, Std deviation: {std_reward}")

    if (mean_reward < 10):
        return []
    else:
        if len(np_data.shape) == 1:
            last_row = np_data  # If there’s only one row
        else:
            last_row = np_data[-1]  # Get last state (latest)

        # Predict on the given state
        obs = np.array(last_row, dtype=np.float32)
        action, _ = model.predict(obs, deterministic=True)
        return action.tolist()

def hyper_tuning():

    # Helper Function
    def make_env():
        env = DashEnv()
        monitored_env = Monitor(env)
        check_env(monitored_env, warn=True)
        return monitored_env

    def objective(trial):
        env = make_env()
        # Suggested hyperparameters
        learning_rate = trial.suggest_loguniform("learning_rate", 1e-5, 1e-3)
        batch_size = trial.suggest_categorical("batch_size", [64, 128, 256])
        buffer_size = trial.suggest_categorical("buffer_size", [10_000, 50_000, 100_000])
        tau = trial.suggest_uniform("tau", 0.005, 0.02)
        gamma = trial.suggest_uniform("gamma", 0.95, 0.9999)
        train_freq = trial.suggest_categorical("train_freq", [1, 4, 8])
        gradient_steps = trial.suggest_categorical("gradient_steps", [1, 4, 8])
        ent_coef = trial.suggest_categorical("ent_coef", ["auto", 0.1, 0.2])
        use_sde = trial.suggest_categorical("use_sde", [False, True])

        model = SAC(
            "MlpPolicy",
            env,
            learning_rate=learning_rate,
            batch_size=batch_size,
            buffer_size=buffer_size,
            tau=tau,
            gamma=gamma,
            train_freq=train_freq,
            gradient_steps=gradient_steps,
            ent_coef=ent_coef,
            use_sde=use_sde,
            verbose=0,
        )

        model.learn(total_timesteps=150) #1200

        # Evaluate
        mean_reward, _ = evaluate_policy(model, env, n_eval_episodes=10, deterministic=True)
        return mean_reward

    study = optuna.create_study(direction="maximize")
    study.optimize(objective, n_trials=10)
    best_params = study.best_params

    return best_params