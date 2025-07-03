import gymnasium as gym
import numpy as np
from gymnasium import spaces


class DashEnv(gym.Env):
    """Custom Environment that follows gym interface."""

    metadata = {"render_modes": ["human"], "render_fps": 30}

    def __init__(self):
        super(DashEnv, self).__init__()
        # Define action and observation space
        # They must be gym.spaces objects
        # Example when using discrete actions:
        self.action_space = spaces.Box(
            low=np.array([1, 1, 60, 0, 1, 1]),  # Min: Num delegates, Delegate Window, Voting Period, Decay factor
            high=np.array([100000, 1000, 31536000, 1, 3600, 1000000]),  # Max: ... min balance, blk create period
            dtype=np.float32
        )

        # Observation Space (environment variables)
        self.observation_space = spaces.Box(
            low=np.array([0, 0, 0, 0, 0, 0]),  # Min: total supply, circulating supply, delegates balance, votes per period,
            high=np.array([1000000000, 1000000000, 100000000, 1000000, 1000000000, 1000000]),  # Max: block height, total Tx Volume
            dtype=np.float32
        )

        # Initialize environment state
        self.state = np.array([500000000, 500000000, 50000000, 500000, 0, 500000], dtype=np.float32)  # Example initial state
        self.current_step = 0

    def step(self, action):
        # Clip and apply actions to governance parameters
        self.num_delegates = int(np.clip(action[0], 1, 100000))
        self.delegate_window = int(np.clip(action[1], 1, 1000))
        self.voting_period = int(np.clip(action[2], 60, 31536000))
        self.decay_factor = float(np.clip(action[3], 0, 1))
        self.min_balance = float(np.clip(action[4], 1, 3600))
        self.block_creation = int(np.clip(action[5], 1, 1000000))

        # Unpacking State
        (
            self.total_supply,
            self.circulating_supply,
            self.delegate_balance,
            self.votes_per_period,
            self.block_height,
            self.tx_volume
        ) = self.state


        # Compute reward based on fairness and efficiency
        reward = self.comp_reward()

        # Define termination condition & Extra info (optional debugging output)
        terminated = self.current_step >= 100  # 'False' True if governance cycle ends
        truncated = False
        info = {}

        # Advance step and return
        obs = np.array([
            self.total_supply,
            self.circulating_supply,
            self.delegate_balance,
            self.votes_per_period,
            self.block_height,
            self.tx_volume
        ], dtype=np.float32)

        # Return correct tuple format (5 values!)
        self.current_step += 1
        return obs, reward, terminated, truncated, info

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)  # Ensures compatibility with Gym's API

        # Reset governance parameters to default
        return self.state, {}

    def comp_reward(self):
        # 1. Num delegates: Scale with circulating supply & votes per period
        ideal_num_delegates = (self.circulating_supply / 1e6) + (self.votes_per_period / 100)
        delegate_score = 1 - abs(self.num_delegates - ideal_num_delegates) / (ideal_num_delegates + 1)

        # 2. Delegate window: Reactivity should align with voting frequency & block height growth
        # As vote activity and chain maturity increase, wider window may be tolerable
        target_window = (self.votes_per_period / 200) + np.log1p(self.block_height)
        window_score = 1 - abs(self.delegate_window - target_window) / (target_window + 1)

        # 3. Voting period: Should shorten with increased participation and growing chain depth
        optimal_period = 7 * 86400 / (1 + self.votes_per_period / 100 + self.block_creation / 1e6)
        voting_score = 1 - abs(self.voting_period - optimal_period) / (optimal_period + 1)

        # 4. Decay factor: Lower decay when the active delegate has a high balance (indicates stability/trust)
        # Inversely favor decay based on deviation from a high balance threshold (e.g., 5% of supply)
        ideal_decay = 1 - min(1.0, self.delegate_balance / (0.05 * self.total_supply + 1e-8))
        decay_score = 1 - abs(self.decay_factor - ideal_decay)

        # 5. Min balance: Should scale with circulating and delegate’s block balance
        target_min_balance = (self.circulating_supply / self.num_delegates + self.delegate_balance) / 2
        balance_score = 1 - abs(self.min_balance - target_min_balance) / (target_min_balance + 1e-8)

        # 6. Block creation period: Should shrink as transaction volume increases
        target_block_time = max(1, 31_536_000 / (self.tx_volume / 1000 + 1))  # prevent div by zero
        block_score = 1 - abs(self.block_creation - target_block_time) / (target_block_time + 1)

        return float(
                0.20 * delegate_score +
                0.15 * window_score +
                0.15 * voting_score +
                0.15 * decay_score +
                0.20 * balance_score +
                0.15 * block_score
        )

    def render(self, mode="human"):
        print(f"Step {self.current_step}:")
        print(f"- Voting Period: {self.voting_period}")
        print(f"- Number of Delegates: {self.num_delegates}")
        print(f"- Stake Variance: {self.stake_variance:.2f}")
        print(f"- Transaction Volume: {self.tx_volume}")
        print("-" * 30)

    def close(self):
        print("Closing DASChain environment...")