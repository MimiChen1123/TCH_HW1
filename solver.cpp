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

int prime_MST(Position &pos, vector<vector<int>> &table_dist){
    vector<Square> reds = BoardView(pos.pieces(Red)).to_vector();
    int n = reds.size();
    if(n == 0){
        return 0;
    }

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    vector<int> min_edge(n, INT32_MAX);
    vector<bool> visited(n, false);

    min_edge[0] = 0;
    pq.push({0, 0});

    while(!pq.empty()){
        int cur = pq.top().second;
        pq.pop();
        
        if(visited[cur]) continue;

        visited[cur] = true;

        for(int i = 0; i < n; i++){
            int dist = table_dist[reds[i]][reds[cur]];
            if(!visited[i] && dist < min_edge[i]){
                min_edge[i] = dist;
                pq.push({dist, i});
            }
        }
    }
    
    int total_mst = 0;
    for(int i = 0; i < n; i++){
        total_mst += min_edge[i];
    }

    return total_mst;
}

void build_dist_table(Position &pos, Square sq, vector<vector<int>> &table_dist){
    queue<pair<Square, int>> q;
    uint64_t visited = 0;
    
    Board ducks = pos.pieces(Black, Duck);
    if(ducks & (1ull << sq)) return;

    visited |= ducks;

    q.push({sq, 0});
    visited |= (1ull << sq);
    table_dist[sq][sq] = 0;
    int dr[4] = {0, 0, 1, -1}; 
    int dc[4] = {1, -1, 0, 0};
    
    while(!q.empty()){
        auto [cur, cur_dist] = q.front();
        q.pop();
        int r = rank_of(cur), c = file_of(cur);
        for(int i = 0; i < 4; i++){
            int nr = r + dr[i], nc = c + dc[i];
            if(nr < 0 || nr >= 4 || nc < 0 || nc >= 8) continue;
            
            Square next_sq = (Square)(nr * 8 + nc);

            if(visited & (1ull << next_sq)) continue;
            
            visited |= (1ull << next_sq);
            table_dist[sq][next_sq] = cur_dist + 1;
            q.push({next_sq, cur_dist + 1});
        }
    }
}

void build_chariot_table(Position &pos, Square sq, vector<vector<int>> &chariot_table){
    queue<pair<Square, int>> q;
    uint64_t visited = 0;
    
    Board ducks = pos.pieces(Black, Duck);
    if(ducks & (1ull << sq)) return;

    visited |= ducks;

    q.push({sq, 0});
    visited |= (1ull << sq);
    chariot_table[sq][sq] = 0;
    int dr[4] = {0, 0, 1, -1}; 
    int dc[4] = {1, -1, 0, 0};
    
    while(!q.empty()){
        auto [cur, cur_dist] = q.front();
        q.pop();
        int r = rank_of(cur), c = file_of(cur);
        for(int i = 0; i < 4; i++){
            int nr = r, nc = c;
            while(true){
                nr += dr[i];
                nc += dc[i];
                if(nr < 0 || nr >= 4 || nc < 0 || nc >= 8) break;

                Square next_sq = (Square)(nr * 8 + nc);
                if(ducks & (1ull << next_sq)) break;
                if(visited & (1ull << next_sq)) continue;

                visited |= (1ull << next_sq);
                chariot_table[sq][next_sq] = cur_dist + 1;
                q.push({next_sq, cur_dist + 1});
            }
        }
    }
}

int find_table_dist(Position &pos, vector<vector<int>> table_dist, vector<vector<int>> &chariot_table){
    vector<Square> blacks = BoardView(pos.pieces(Black)).to_vector();
    vector<Square> reds = BoardView(pos.pieces(Red)).to_vector();
    int dist = INT32_MAX;
    for(auto bq: blacks){
        Piece bp = pos.peek_piece_at(bq);
        for(auto rq: reds){
            Piece rp = pos.peek_piece_at(rq);
            if(bp.type == Duck) continue;
            if(bp.type == Chariot){
                dist = min(dist, (table_dist[bq][rq] / chariot_table[bq][rq]));
            }
            else{
                dist = min(dist, table_dist[bq][rq]);
            }
        }
    }
    
    return dist;
}

int find_table_mst(Position &pos, unordered_map<uint32_t, int> &table_mst, vector<vector<int>> &table_dist){
    Board red_pieces = pos.pieces(Red);
    int mst = -1;
    if(table_mst.find(red_pieces) != table_mst.end()){
        mst = table_mst[red_pieces];
    }
    else{
        mst = prime_MST(pos, table_dist);
        table_mst[red_pieces] = mst;
    }
    return mst;
}

void resolve(Position &pos)
{
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    // info << pos;
    /* A random number generator is available globally */
    int random_num_below_42 = rng(42);

    vector<vector<int>> table_dist(32, vector<int>(32, INT32_MAX));
    vector<vector<int>> chariot_table(32, vector<int>(32, INT32_MAX));
    Board all_pieces = (~pos.pieces()) | (pos.pieces(Red)) | (pos.pieces(Black));
    all_pieces &= (~pos.pieces(Black, Duck));

    for(int sq = 0; sq < 32; sq++){
        build_dist_table(pos, (Square)sq, table_dist);
    }
    for(int sq = 0; sq < 32; sq++){
        build_chariot_table(pos, (Square)sq, chariot_table);
    }
    
    unordered_map<uint32_t, int> table_mst;
    priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;
    unordered_map<string, int> visited;
    unordered_map<string, string> predecessor;
    unordered_map<string, Move> moves;
    int mst = find_table_mst(pos, table_mst, table_dist);
    int dist = find_table_dist(pos, table_dist, chariot_table);
    int f_val = mst + dist + 0;
    string start = pos.toFEN();
    visited[start] = 0;
    predecessor[start] = start;
    pq.push({f_val, start});

    while(!pq.empty()){
        string cur_key = pq.top().second;
        Position cur(cur_key);
        pq.pop();
        
        if(cur.winner() == Black){
            clock_gettime(CLOCK_REALTIME, &end_time);
            double wall_clock_in_seconds =(double)((end_time.tv_sec + end_time.tv_nsec * (1e-9))
                                             - (double)(start_time.tv_sec + start_time.tv_nsec * (1e-9)));
    
            info << wall_clock_in_seconds << "\n";
            info << visited[cur_key] << "\n";

            string tmp = cur_key;
            vector<Move> ans;
            while(tmp != predecessor[tmp]){
                ans.push_back(moves[tmp]);
                tmp = predecessor[tmp];
            }
            reverse(ans.begin(), ans.end());
            for (auto mv : ans) {
                info << mv;
            }
            return;
        }

        MoveList mvs(cur);

        for(Move mv: mvs){
            Position next_pos(cur);
            if(!next_pos.do_move(mv)) continue;
            for(int sq = 0; sq < 32; sq++){
                build_dist_table(next_pos, (Square)sq, table_dist);
            }
            for(int sq = 0; sq < 32; sq++){
                build_chariot_table(next_pos, (Square)sq, chariot_table);
            }
            
            string next_key = next_pos.toFEN();
            int cost = visited[cur_key] + 1;
            if(visited.find(next_key) == visited.end() || cost < visited[next_key]){
                mst = find_table_mst(next_pos, table_mst, table_dist);
                dist = find_table_dist(next_pos, table_dist, chariot_table);
                visited[next_key] = cost;
                predecessor[next_key] = cur_key;
                moves[next_key] = mv;
                f_val = cost + dist + mst;
                pq.push({f_val, next_key});
            }
        }
        
    }

}