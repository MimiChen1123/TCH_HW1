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

int heuristic(Position &pos){
    Board red_pieces = pos.pieces(Red);
    return BoardView(red_pieces).to_vector().size();
}

void resolve(Position &pos)
{
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

    info << pos;

    /* A random number generator is available globally */
    int random_num_below_42 = rng(42);

    string start = pos.toFEN();
    priority_queue<pair<int, string>, vector<pair<int, string>>, cmp> pq;
    pq.push({heuristic(pos), start});

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

                pq.push({cost + heuristic(copy_pos), key});
            }
        }
        
    }

}