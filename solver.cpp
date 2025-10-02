#include "solver.h"
#include "lib/helper.h"
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_set>
#include <string>
#include <time.h>
using namespace std;

/*
// 1. 怎麼執行？我需要處理 input 嗎？
2. do_move 到底是做什麼？他回傳的是代表是什麼意思？
3. distance<File> 是什麼意思？
4. 把 Red 吃掉這個行為是我需要去實作的，還是呼叫哪一個 function 即可？
5. 我 pos 是什麼時候更新的？
6. Invalid moves 是什麼意思？我自己跑看起來很合理呀？
*/


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

struct Status {
    Position pos;
    int g;
    int h;
    vector<Move> path;
    int f() const {
        return g + h;
    }

    bool operator>(const Status &other) const {
        return f() > other.f();
    }
};

int heuristic(Position &pos){
    Board red_pieces = pos.pieces(Red);
    // return __builtin_popcount(red_pieces);
    return BoardView(red_pieces).to_vector().size();
}

void resolve(Position &pos)
{
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    /* You can use these aliases */
    // info << "Output (only) your answers here!\n"; // ====> stdout <====
    // debug << "Print debug info here!\n";          // stderr
    // error << "Print errors here?\n";              // also stderr

    // info << pos;

    /* A random number generator is available globally */
    int random_num_below_42 = rng(42);

    Status start = {pos, 0, heuristic(pos), {}};
    priority_queue<Status, vector<Status>, greater<Status>> pq;
    pq.push(start);

    unordered_set<string> visited;
    visited.insert(pos.toFEN());

    while(!pq.empty()){
        Status cur = pq.top();
        pq.pop();

        if(cur.pos.winner() == Black){
            clock_gettime(CLOCK_REALTIME, &end_time);
            double wall_clock_in_seconds =(double)((end_time.tv_sec + end_time.tv_nsec * (1e-9))
                                             - (double)(start_time.tv_sec + start_time.tv_nsec * (1e-9)));
    
            info << wall_clock_in_seconds << "\n";
            info << cur.g << "\n";
            int count = 1;
            for(Move mv: cur.path){
                info << mv;
                count++;
            }
            return;
        }

        MoveList mvs(cur.pos);

        for(Move mv: mvs){
            Position copy_pos(cur.pos);
            if(!copy_pos.do_move(mv)){
                continue;
            }

            string key = copy_pos.toFEN();
            if(visited.find(key) != visited.end()){
                continue;
            }
            
            cur.path.push_back(mv);
            Status next = {copy_pos, cur.g + 1, heuristic(copy_pos), cur.path};
            pq.push(move(next));
            visited.insert(key);
            cur.path.pop_back();
        }
        
    }

}