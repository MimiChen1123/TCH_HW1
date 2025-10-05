#include "solver.h"
#include "lib/helper.h"
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <time.h>
#include <typeinfo>
using namespace std;


/*
 * Wakasagi will call this and only this function.
 * Below you will find examples of functions you might use
 * and an example for outputting random moves.
 *
 * The header files contain detailed comments.
 *
 * Guide & documentation can be found here:
 * https://docs.google.com/document/d/1AcbayzQvl0wyp6yl5XzhtDRPyOxINTpjR-1ghtxHQso/edit?usp=sharing
 *
 * Good luck!
 */

const int dr[4] = {0, 0, 1, -1}; 
const int dc[4] = {1, -1, 0, 0};

int bfs_general(Position &pos, Square start, Square end){
    queue<pair<Square, int>> q;
    Board all_pieces = pos.pieces();
    uint64_t obstacle = all_pieces;
    obstacle &= ~(1ull << start);
    obstacle &= ~(1ull << end);

    uint64_t visited = 0;
    visited |= (1ull << start);
    q.push({start, 0});

    while(!q.empty()){
        auto [cur, dist] = q.front();
        q.pop();

        int r = rank_of(cur), c = file_of(cur);
        for(int i = 0; i < 4; i++){
            int nr = r + dr[i], nc = c + dc[i];
            if(nr < 0 || nr >= 4 || nc < 0 || nc >= 8) continue;
            
            Square next_sq = (Square)(nr * 8 + nc);
            uint64_t next_mask = 1ull << next_sq;

            if((obstacle & next_mask) || (visited & next_mask)) continue;
            
            if(next_sq == end){
                return dist + 1;
            }
            visited |= next_mask;
            q.push({next_sq, dist + 1});
        }
    }

    return INT32_MAX;
}

int bfs_chariot(Position &pos, Square start, Square end){
    queue<pair<Square, int>> q;
    Board all_pieces = pos.pieces();
    uint32_t obstacle = all_pieces;
    obstacle &= ~(1ull << start);
    obstacle &= ~(1ull << end);

    uint32_t visited = 0;
    visited |= (1ull << start);
    q.push({start, 0});

    while(!q.empty()){
        auto [cur, dist] = q.front();
        q.pop();

        int r = rank_of(cur), c = file_of(cur);
        for(int i = 0; i < 4; i++){
            int nr = r, nc = c;
            while(true){
                nr += dr[i];
                nc += dc[i];
                if(nr < 0 || nr >= 4 || nc < 0 || nc >= 8) break;

                Square next_sq = (Square)(nr * 8 + nc);
                uint32_t next_mask = 1ull << next_sq;

                if (obstacle & next_mask) break;

                if (visited & next_mask) continue;
                
                if(next_sq == end){
                    return dist + 1;
                }
                visited |= next_mask;
                q.push({next_sq, dist + 1});
            }
        }
    }

    return INT32_MAX;
}

int find_table_dist(Position &pos){
    vector<Square> blacks = BoardView(pos.pieces(Black)).to_vector();
    vector<Square> reds = BoardView(pos.pieces(Red)).to_vector();
    if(reds.size() == 0){
        return 0;
    }

    int dist = INT32_MAX;
    for(auto bp: blacks){
        Piece p = pos.peek_piece_at(bp);
        for(auto rp: reds){
            if(p.type == Duck) continue;
            if(p.type == Chariot){
                dist = min(dist, bfs_chariot(pos, bp, rp));
            }
            else{
                dist = min(dist, bfs_general(pos, bp, rp));
            }
        }
    }
    
    return dist;
}


int dfs(Position &pos, int g, int threshold, vector<Move> &path, unordered_map<uint32_t, int> &table_mst, unordered_map<string, int> &TT) {
    int reds = BoardView(pos.pieces(Red)).to_vector().size();
    int h = reds + find_table_dist(pos);
    int f = g + h;
    if (f > threshold) return f;
    if (pos.winner() == Black) return -1;

    string key = pos.toFEN();
    if (TT.find(key) != TT.end() && TT[key] <= g) return INT32_MAX;
    TT[key] = g;

    int min_next = INT32_MAX;
    MoveList mvs(pos);
    for (Move mv : mvs) {
        Position next_pos(pos);
        if (!next_pos.do_move(mv)) continue;
        path.push_back(mv);
        int t = dfs(next_pos, g + 1, threshold, path, table_mst, TT);
        if (t == -1) return -1;
        path.pop_back();
        if (t < min_next) min_next = t;
    }
    return min_next;
}

void resolve(Position &pos) {
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    int random_num_below_42 = rng(42);
    // info << pos;
    unordered_map<uint32_t, int> table_mst;
    int reds = BoardView(pos.pieces(Red)).to_vector().size();
    int threshold = reds + find_table_dist(pos);
    unordered_map<string, int> TT;
    while (true) {
        vector<Move> path;
        int t = dfs(pos, 0, threshold, path, table_mst, TT);
        if (t == -1){
            clock_gettime(CLOCK_REALTIME, &end_time);
            double wall_clock_in_seconds =(double)((end_time.tv_sec + end_time.tv_nsec * (1e-9))
                                             - (double)(start_time.tv_sec + start_time.tv_nsec * (1e-9)));

            info << wall_clock_in_seconds << "\n";
            info << path.size() << "\n";

            for(Move mv: path){
                info << mv;
            }
            return;
        }
        threshold = t;
        TT.clear();
    }
}
