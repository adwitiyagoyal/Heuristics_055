#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>

using namespace std;

const int N = 200;

// Input data
long long H_orig[N];
int C_orig[N];
int A[N][N];

// State variables
long long hardness[N];
int durability[N];
bool is_open[N];
int num_open_boxes = 0;

vector<pair<int, int>> result_attacks;

// Helper function to perform a weapon attack and update state
void perform_weapon_attack(int w, int b) {
    result_attacks.push_back({w, b});
    hardness[b] -= A[w][b];
    durability[w]--;

    if (!is_open[b] && hardness[b] <= 0) {
        is_open[b] = true;
        num_open_boxes++;
    }
}

// Helper function to perform a bare-hand unlock and update state
void perform_bare_hand_unlock(int b) {
    if (hardness[b] > 0) {
        long long attacks_needed = hardness[b];
        for (long long i = 0; i < attacks_needed; ++i) {
            result_attacks.push_back({-1, b});
        }
        hardness[b] -= attacks_needed;
    }
    
    if (!is_open[b]) {
        is_open[b] = true;
        num_open_boxes++;
    }
}

int main() {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    int n_dummy;
    cin >> n_dummy;

    for (int i = 0; i < N; ++i) cin >> H_orig[i];
    for (int i = 0; i < N; ++i) cin >> C_orig[i];
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            cin >> A[i][j];
        }
    }

    // Initialize state
    for (int i = 0; i < N; ++i) {
        hardness[i] = H_orig[i];
        durability[i] = C_orig[i];
        is_open[i] = false;
    }

    while (num_open_boxes < N) {
        // Part 1: Find the best weapon attack
        int best_w = -1, best_b = -1;

        int best_finishing_w = -1, best_finishing_b = -1, min_finishing_A = 2001;
        int best_progress_w = -1, best_progress_b = -1, max_progress_A = 0;

        for (int w = 0; w < N; ++w) {
            if (is_open[w] && durability[w] > 0) {
                for (int b = 0; b < N; ++b) {
                    if (!is_open[b]) {
                        if (hardness[b] > 0 && A[w][b] >= hardness[b]) {
                            if (A[w][b] < min_finishing_A) {
                                min_finishing_A = A[w][b];
                                best_finishing_w = w;
                                best_finishing_b = b;
                            }
                        }
                        if (A[w][b] > max_progress_A) {
                            max_progress_A = A[w][b];
                            best_progress_w = w;
                            best_progress_b = b;
                        }
                    }
                }
            }
        }
        
        if (best_finishing_w != -1) {
            best_w = best_finishing_w;
            best_b = best_finishing_b;
        } else if (best_progress_w != -1) {
            best_w = best_progress_w;
            best_b = best_progress_b;
        }

        if (best_w != -1) {
            perform_weapon_attack(best_w, best_b);
        } else {
            // Part 2: No usable weapon, must use bare hands
            int box_to_unlock = -1;
            double max_score = -1.0;

            for (int b = 0; b < N; ++b) {
                if (!is_open[b]) {
                    if (hardness[b] <= 0) {
                        box_to_unlock = b;
                        break; 
                    }
                    
                    vector<long long> gains;
                    for (int target_b = 0; target_b < N; ++target_b) {
                        if (!is_open[target_b] && target_b != b) {
                            if (A[b][target_b] > 1) {
                                gains.push_back(A[b][target_b] - 1);
                            }
                        }
                    }

                    long long potential = 0;
                    int k = C_orig[b];
                    if (!gains.empty()) {
                        if (k >= gains.size()) {
                            potential = accumulate(gains.begin(), gains.end(), 0LL);
                        } else {
                            nth_element(gains.begin(), gains.begin() + k - 1, gains.end(), greater<long long>());
                            potential = accumulate(gains.begin(), gains.begin() + k, 0LL);
                        }
                    }
                    
                    double score = (double)potential / hardness[b];
                    if (score > max_score) {
                        max_score = score;
                        box_to_unlock = b;
                    }
                }
            }
            perform_bare_hand_unlock(box_to_unlock);
        }
    }

    // Print the sequence of attacks
    for (const auto& p : result_attacks) {
        cout << p.first << " " << p.second << "\n";
    }

    return 0;
}