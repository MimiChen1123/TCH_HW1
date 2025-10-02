#include "solver.h"
#include "lib/helper.h"
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
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


struct cmp {
    bool operator()(const pair<int, string> a, const pair<int, string> b) const {
        return a.first > b.first;
    }
};

// int prime_MST(Position &pos) {

//     Board red_pieces = pos.pieces(Red);
//     Board black_pieces = pos.pieces(Black);

//     vector<Square> reds = BoardView(red_pieces).to_vector();
//     vector<Square> blacks = BoardView(black_pieces).to_vector();

//     if (reds.empty()) return 0;

//     vector<Square> nodes;
//     nodes.push_back(Square());
//     for (Square rp : reds) nodes.push_back(rp);

//     int n = nodes.size();
//     vector<int> min_edge(n, INT32_MAX);

//     min_edge[0] = 0;

//     for (int i = 1; i < n; i++) {
//         int best = INT32_MAX;
//         for (Square bp : blacks) {
//             Piece p = pos.peek_piece_at(bp);
//             if (p.type == Duck){
//                 continue;
//             }
//             if (p.type == Chariot){
//                 int c = (distance<File>(bp, nodes[i]) > 0)? 1 : 0;
//                 int r = (distance<Rank>(bp, nodes[i]) > 0)? 1 : 0;
//                 best = min(best, c + r);
//             }
//             else{
//                 best = min(best, distance(bp, nodes[i]));
//             }
            
//         }
//         min_edge[i] = best;
//     }

//     priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
//     vector<bool> visited(n, false);


//     for(int i = 0; i < n; i++){
//         pq.push({min_edge[i], i});
//     }
//     int total_mst = 0;
//     while(!pq.empty()){
//         pair<int, int> cur = pq.top();
//         int cur_node = cur.second;
//         pq.pop();
//         if(visited[cur_node]){
//             continue;
//         }
//         total_mst += cur.first;
//         visited[cur_node] = true;

//         for(int i = 0; i < n; i++){
//             int dist = distance(nodes[cur_node], nodes[i]);
//             if(!visited[i] && dist < min_edge[i]){
//                 min_edge[i] = dist;
//                 pq.push({dist, i});
//             }
//         }
//     }

//     int tmp = 0;
//     for(int i = 0; i < n; i++){
//         tmp += min_edge[i];
//     }
//     if(tmp == total_mst){

//     }
//     return total_mst;
// }


int prime_MST(const vector<Square> reds) {
    int n = reds.size();

    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
    vector<int> min_edge(n, INT32_MAX);
    vector<bool> visited(n, false);
    min_edge[0] = 0;
    pq.push({0, 0});

    int total_mst = 0;

    while(!pq.empty()){
        pair<int, int> cur = pq.top();
        int cur_node = cur.second;
        pq.pop();
        
        if(visited[cur_node]){
            continue;
        }

        total_mst += cur.first;
        visited[cur_node] = true;

        for(int i = 0; i < n; i++){
            int dist = distance(reds[cur_node], reds[i]);
            if(!visited[i] && dist < min_edge[i]){
                min_edge[i] = dist;
                pq.push({dist, i});
            }
        }
    }

    return total_mst;
}


int heuristic(Position &pos, unordered_map<string, int> &table_mst){
    string key = pos.toFEN();

    Board red_pieces = pos.pieces(Red);
    Board black_pieces = pos.pieces(Black);

    vector<Square> reds = BoardView(red_pieces).to_vector();
    vector<Square> blacks = BoardView(black_pieces).to_vector();

    if (reds.empty()) return 0;

    int mst;
    if(table_mst.find(key) == table_mst.end()){
        mst = prime_MST(reds);
        table_mst[key] = mst;
    }
    else{
        mst = table_mst[key];
    }

    int nearest = INT32_MAX;
    for (auto bp : blacks) {
        Piece p = pos.peek_piece_at(bp);
        if(p.type == Duck){
            continue;
        }
        for (auto rp : reds) {
            if (p.type == Chariot) {
                int r = (distance<Rank>(bp, rp) > 0)? 1 : 0;
                int c = (distance<File>(bp, rp) > 0)? 1 : 0;
                nearest = min(nearest, r + c);
            }
            else{
                nearest = min(nearest, distance(bp, rp));
            }
            
        }
    }
    

    return mst + nearest + (reds.size() - 1);
}

void resolve(Position &pos)
{
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    // info << pos;

    /* A random number generator is available globally */
    int random_num_below_42 = rng(42);
    string start = pos.toFEN();
    unordered_map<string, int> table_mst;
    priority_queue<pair<int, string>, vector<pair<int, string>>, cmp> pq;
    pq.push({heuristic(pos, table_mst), start});

    unordered_map<string, int> visited; // key: fen, value: moves(g)
    unordered_map<string, string> path; // key: cur.fen , value: prev.fen
    unordered_map<string, Move> moves; //key: fen, value: move

    visited[start] = 0;
    path[start] = start;

    while(!pq.empty()){
        pair<int, string> cur = pq.top();
        string cur_key = cur.second;
        pq.pop();
        
        Position cur_pos(cur_key);
        
        if(cur_pos.winner() == Black){
            clock_gettime(CLOCK_REALTIME, &end_time);
            double wall_clock_in_seconds =(double)((end_time.tv_sec + end_time.tv_nsec * (1e-9))
                                             - (double)(start_time.tv_sec + start_time.tv_nsec * (1e-9)));
    
            info << wall_clock_in_seconds << "\n";
            info << visited[cur_key] << "\n";

            string tmp = cur_key;
            vector<Move> ans;
            while(tmp != path[tmp]){
                ans.push_back(moves[tmp]);
                tmp = path[tmp];
            }
            reverse(ans.begin(), ans.end());
            for (auto mv : ans) {
                info << mv;
            }
            return;
        }

        MoveList mvs(cur_pos);

        for(Move mv: mvs){
            Position copy_pos(cur_pos);
            if(!copy_pos.do_move(mv)){
                continue;
            }
            
            string key = copy_pos.toFEN();
            int cost = visited[cur_key] + 1;
            if(visited.find(key) == visited.end() || cost < visited[key]){
                visited[key] = cost;
                path[key] = cur_key;
                moves[key] = mv;

                pq.push({cost + heuristic(copy_pos, table_mst), key});
            }
        }
        
    }

}