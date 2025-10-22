#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <random>

using namespace std;

// --- Global Constants and Utilities ---
const int N = 200;
auto RND = mt19937(chrono::steady_clock::now().time_since_epoch().count());
const auto T_START = chrono::steady_clock::now();

// --- Input Data ---
long long H_orig[N];
int C_orig[N];
int A[N][N];

// --- Solution Structure ---
struct Solution {
    vector<pair<int, int>> attacks;
    long long score = -1;
};

// --- Core Greedy Solver ---
// Generates a full plan based on a high-level unlock_order strategy
Solution generate_plan(const vector<int>& unlock_order) {
    long long hardness[N];
    int durability[N];
    bool is_open[N] = {false};
    int num_open_boxes = 0;
    
    for(int i=0; i<N; ++i) {
        hardness[i] = H_orig[i];
        durability[i] = C_orig[i];
    }

    Solution sol;
    size_t unlock_idx = 0;

    while (num_open_boxes < N) {
        // Part 1: Greedily use weapons as long as possible
        while (true) {
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
                best_w = best_finishing_w; best_b = best_finishing_b;
            } else if (best_progress_w != -1) {
                best_w = best_progress_w; best_b = best_progress_b;
            }

            if (best_w != -1) {
                sol.attacks.push_back({best_w, best_b});
                hardness[best_b] -= A[best_w][best_b];
                durability[best_w]--;
                if (!is_open[best_b] && hardness[best_b] <= 0) {
                    is_open[best_b] = true;
                    num_open_boxes++;
                }
            } else {
                break;
            }
        }
        
        if (num_open_boxes == N) break;

        // Part 2: Perform the next bare-hand unlock from the strategic order
        int box_to_unlock = -1;
        
        while(unlock_idx < unlock_order.size() && is_open[unlock_order[unlock_idx]]){
            unlock_idx++;
        }
        if(unlock_idx < unlock_order.size()){
            box_to_unlock = unlock_order[unlock_idx];
        } else {
            for(int b=0; b<N; ++b) if(!is_open[b]) {box_to_unlock=b; break;}
        }

        if (hardness[box_to_unlock] > 0) {
            long long attacks_needed = hardness[box_to_unlock];
            for (long long i = 0; i < attacks_needed; ++i) sol.attacks.push_back({-1, box_to_unlock});
            hardness[box_to_unlock] -= attacks_needed;
        }
        if (!is_open[box_to_unlock]) {
            is_open[box_to_unlock] = true;
            num_open_boxes++;
        }
    }
    sol.score = sol.attacks.size();
    return sol;
}


// --- Main Simulated Annealing Logic ---
int main() {
    ios_base::sync_with_stdio(false); cin.tie(NULL);

    int n_dummy; cin >> n_dummy;
    for (int i = 0; i < N; ++i) cin >> H_orig[i];
    for (int i = 0; i < N; ++i) cin >> C_orig[i];
    for (int i = 0; i < N; ++i) for (int j = 0; j < N; ++j) cin >> A[i][j];

    // --- Generate Initial Unlock Strategy ---
    vector<int> initial_unlock_order;
    {
        long long hardness[N]; for(int i=0; i<N; ++i) hardness[i] = H_orig[i];
        bool is_open[N] = {false};
        int num_open_boxes = 0;
        while(num_open_boxes < N){
            int box_to_unlock = -1;
            double max_score = -1.0;
            for(int b=0; b<N; ++b){
                if(!is_open[b]){
                    vector<long long> gains;
                    for(int t=0; t<N; ++t) if(t!=b) if(A[b][t]>1) gains.push_back(A[b][t]-1);
                    long long potential = 0;
                    int k = C_orig[b];
                    if(!gains.empty()){
                        if((size_t)k >= gains.size()) potential=accumulate(gains.begin(), gains.end(), 0LL);
                        else {
                            nth_element(gains.begin(), gains.begin()+k-1, gains.end(), greater<long long>());
                            potential=accumulate(gains.begin(), gains.begin()+k, 0LL);
                        }
                    }
                    double score = (double)potential / hardness[b];
                    if(score > max_score){max_score=score; box_to_unlock=b;}
                }
            }
            initial_unlock_order.push_back(box_to_unlock);
            is_open[box_to_unlock] = true;
            num_open_boxes++;
        }
    }

    vector<int> current_order = initial_unlock_order;
    Solution current_sol = generate_plan(current_order);
    Solution best_sol = current_sol;

    double T_start_val = 500.0, T_end_val = 0.1;

    while(true){
        auto T_now = chrono::steady_clock::now();
        double time_elapsed = chrono::duration<double>(T_now - T_START).count();
        if(time_elapsed > 1.95) break;

        vector<int> next_order = current_order;
        
        // ================== IMPROVED MUTATION LOGIC ==================
        // Swap two random positions in the unlock order. This is a much
        // more effective way to explore neighboring solutions.
        uniform_int_distribution<> distrib(0, N-1);
        int idx1 = distrib(RND);
        int idx2 = distrib(RND);
        if (idx1 != idx2) {
            swap(next_order[idx1], next_order[idx2]);
        }
        // =============================================================

        Solution next_sol = generate_plan(next_order);

        double temp = T_start_val * pow(T_end_val / T_start_val, time_elapsed / 1.95);
        double prob = exp((double)(current_sol.score - next_sol.score) / temp);

        uniform_real_distribution<> prob_dist(0.0, 1.0);
        if (prob > prob_dist(RND)) {
            current_sol = next_sol;
            current_order = next_order;
        }

        if (current_sol.score < best_sol.score) {
            best_sol = current_sol;
        }
    }

    for (const auto& p : best_sol.attacks) {
        cout << p.first << " " << p.second << "\n";
    }

    return 0;
}