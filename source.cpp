#include <bits/stdc++.h>
#include "png.h"

using namespace std;

void operator+=(pair<int,int> &lhs, const pair<int,int> rhs){
    lhs.first += rhs.first;
    lhs.second += rhs.second;
}

char file_name[] = "shuffled-images-data/data_train/64-sources/1236.png";
// char file_name[] = "shuffled-images-data/data_test2_blank/32/3000.png";
const int P = 64;
const int M = 512/P;
const int N = M*M;
const int LM = P/16;
ofstream fout("out.txt");

void abort_(const char * s, ...)
{
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;

void read_png_file(char* file_name)
{
        uint8_t header[8];    // 8 is the maximum size that can be checked

        /* open file and test for it being a png */
        FILE *fp = fopen(file_name, "rb");
        if (!fp)
                abort_("[read_png_file] File %s could not be opened for reading", file_name);
        int z = fread(header, 1, 8, fp);
        if (png_sig_cmp(header, 0, 8))
                abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);


        /* initialize stuff */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                abort_("[read_png_file] png_create_read_struct failed");

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
                abort_("[read_png_file] png_create_info_struct failed");

        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[read_png_file] Error during init_io");

        png_init_io(png_ptr, fp);
        png_set_sig_bytes(png_ptr, 8);

        png_read_info(png_ptr, info_ptr);

        width = png_get_image_width(png_ptr, info_ptr);
        height = png_get_image_height(png_ptr, info_ptr);
        color_type = png_get_color_type(png_ptr, info_ptr);
        bit_depth = png_get_bit_depth(png_ptr, info_ptr);

        number_of_passes = png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);


        /* read file */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[read_png_file] Error during read_image");

        row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
        for (int y=0; y<height; y++)
                row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

        png_read_image(png_ptr, row_pointers);

        fclose(fp);
}

// preprocessed
array<array<array<uint8_t,3>,512>,512> pixel;
vector<vector<vector<int>>> zz;
vector<vector<vector<int>>> es;

// strat2 only
vector<vector<int>> penalty;

void process_file(){
    for (int y=0; y<height; y++) {
            png_byte* row = row_pointers[y];
            for (int x=0; x<width; x++) {
                    png_byte* ptr = &(row[x*3]);
                    //printf("Pixel at position [ %d - %d ] has RGB values: %d - %d - %d\n",
                    //        x, y, ptr[0], ptr[1], ptr[2]);
                    pixel[y][x][0] = ptr[0];
                    pixel[y][x][1] = ptr[1];
                    pixel[y][x][2] = ptr[2];
            }
    }
}

int pixel_diff(const array<array<array<uint8_t,3>,512>,512> &pixel, pair<int,int> a, pair<int,int> b){
    const int aR = pixel[a.first][a.second][0];
    const int aB = pixel[a.first][a.second][1];
    const int aG = pixel[a.first][a.second][2];
    const int bR = pixel[b.first][b.second][0];
    const int bB = pixel[b.first][b.second][1];
    const int bG = pixel[b.first][b.second][2];
    return (aR-bR)*(aR-bR) + (aB-bB)*(aB-bB) + (aG-bG)*(aG-bG);
    //return min((aR-bR)*(aR-bR), 100) + min((aB-bB)*(aB-bB), 100) + min((aG-bG)*(aG-bG), 100);
    // return abs(aR-bR) + abs(aB-bB) + abs(aG-bG);
}

vector<vector<vector<int>>> compute_es(){
    es = vector<vector<vector<int>>>(N, vector<vector<int>>(N, vector<int>(4,0)));
    // ee = vector<vector<vector<double>>>(N, vector<vector<double>>(N, vector<double>(4,0)));
    for(int a=0;a<N;a++){
    for(int b=0;b<N;b++){
        // A : pixel[(i/M)*P, (i%M)*P], PxP
        // B : pixel[(j/M)*P, (j%M)*P], PxP
        pair<int,int> aTR = { (a/M)*P , (a%M)*P };
        pair<int,int> bTR = { (b/M)*P , (b%M)*P };

        // [A][B]
        pair<int,int> a1 = aTR;
        pair<int,int> b1 = bTR;
        pair<int,int> a1_prev, b1_prev; // a1_prev
        pair<int,int> a1_pp, b1_pp; // a1_prev_prev
        
        a1 += make_pair(0 , P-1);
        for(int k = 0; k < P; k++){
            // pair<int,int> a1_adj = a1; a1_adj += { 0, -1 };
            // pair<int,int> b1_adj = b1; b1_adj += { 0, 1 };
            // pair<int,int> a1_p_adj = a1_prev; a1_p_adj += { 0, -1 };
            // pair<int,int> b1_p_adj = b1_prev; b1_p_adj += { 0, 1 };

            // es[a][b][0] += pixel_diff(pixel, a1, b1);
            if(k == 1){
                es[a][b][0] += min(
                    pixel_diff(pixel, a1_prev, b1_prev),
                    pixel_diff(pixel, a1_prev, b1)
                );
                es[a][b][0] += min(
                    pixel_diff(pixel, b1_prev, a1_prev),
                    pixel_diff(pixel, b1_prev, a1)
                );
            }
            if(k >= 2){
                es[a][b][0] += min(min(
                    pixel_diff(pixel, a1_prev, b1),
                    pixel_diff(pixel, a1_prev, b1_prev)),
                    pixel_diff(pixel, a1_prev, b1_pp)
                );
                es[a][b][0] += min(min(
                    pixel_diff(pixel, b1_prev, a1),
                    pixel_diff(pixel, b1_prev, a1_prev)),
                    pixel_diff(pixel, b1_prev, a1_pp)
                );
            }
            if(k == P-1){
                es[a][b][0] += min(
                    pixel_diff(pixel, a1, b1_prev),
                    pixel_diff(pixel, a1, b1)
                );
                es[a][b][0] += min(
                    pixel_diff(pixel, b1, a1_prev),
                    pixel_diff(pixel, b1, a1)
                );
            }
            a1_pp = a1_prev; b1_pp = b1_prev;
            a1_prev = a1; b1_prev = b1;
            a1 += make_pair(1, 0);
            b1 += make_pair(1, 0);
        }

        // [A]
        // [B]
        pair<int,int> a2 = aTR;
        pair<int,int> b2 = bTR;
        pair<int,int> a2_prev, b2_prev; // a1_prev
        pair<int,int> a2_pp, b2_pp; // a1_prev_prev

        a2 += make_pair(P-1 , 0);
        for(int k = 0; k < P; k++){
            //es[a][b][1] += pixel_diff(pixel, a2, b2);
            if(k == 1){
                es[a][b][1] += min(
                    pixel_diff(pixel, a2_prev, b2_prev),
                    pixel_diff(pixel, a2_prev, b2)
                );
                es[a][b][1] += min(
                    pixel_diff(pixel, b2_prev, a2_prev),
                    pixel_diff(pixel, b2_prev, a2)
                );
            }
            if(k >= 2){
                es[a][b][1] += min(min(
                    pixel_diff(pixel, a2_prev, b2),
                    pixel_diff(pixel, a2_prev, b2_prev)),
                    pixel_diff(pixel, a2_prev, b2_pp)
                );
                es[a][b][1] += min(min(
                    pixel_diff(pixel, b2_prev, a2),
                    pixel_diff(pixel, b2_prev, a2_prev)),
                    pixel_diff(pixel, b2_prev, a2_pp)
                );
            }
            if(k == P-1){
                es[a][b][1] += min(
                    pixel_diff(pixel, a2, b2_prev),
                    pixel_diff(pixel, a2, b2)
                );
                es[a][b][1] += min(
                    pixel_diff(pixel, b2, a2_prev),
                    pixel_diff(pixel, b2, a2)
                );
            }
            a2_pp = a2_prev; b2_pp = b2_prev;
            a2_prev = a2; b2_prev = b2;
            a2 += make_pair(0, 1);
            b2 += make_pair(0, 1);
        } 

        es[b][a][2] = es[a][b][0];
        es[b][a][3] = es[a][b][1];
    }}
    return es;
}

// vector<vector<vector<int>>> compute_es1(const array<array<array<uint8_t,3>,512>,512> &pixel){
//     vector<vector<vector<int>>> es(N, vector<vector<int>>(N, vector<int>(4,0)));
//     for(int a=0;a<N;a++){
//     for(int b=0;b<N;b++){
//         // A : pixel[(i/M)*P, (i%M)*P], PxP
//         // B : pixel[(j/M)*P, (j%M)*P], PxP
//         pair<int,int> aTR = { (a/M)*P , (a%M)*P };
//         pair<int,int> bTR = { (b/M)*P , (b%M)*P };

//         // [A][B]
//         pair<int,int> a1 = aTR;
//         pair<int,int> b1 = bTR;
//         a1 += make_pair(0 , P-1);
//         for(int k = 0; k < P; k++){
//             es[a][b][0] += pixel_diff(pixel, a1, b1);
//             a1 += make_pair(1, 0);
//             b1 += make_pair(1, 0);
//         }

//         // [A]
//         // [B]
//         pair<int,int> a2 = aTR;
//         pair<int,int> b2 = bTR;
//         a2 += make_pair(P-1 , 0);
//         for(int k = 0; k < P; k++){
//             es[a][b][1] += pixel_diff(pixel, a2, b2);
//             a2 += make_pair(0, 1);
//             b2 += make_pair(0, 1);
//         } 

//         es[b][a][2] = es[a][b][0];
//         es[b][a][3] = es[a][b][1];
//     }}
//     return es;
// }

const vector<pair<int,int>> dir = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
const vector<int> dirR = {0, 1, 0, -1};
const vector<int> dirC = {1, 0, -1, 0};


int r_min=INT_MAX,r_max=0,c_min=INT_MAX,c_max=0;

vector<vector<int>> T(4*M, vector<int>(4*M, -1));
vector<bool> used(N, false);

void place(int t, int r, int c){
    cout << "place(" << t << "," << r << "," << c << ")" << endl;
    T[r][c] = t;
    used[t] = true;

    r_min = min(r_min, r);
    r_max = max(r_max, r);
    c_min = min(c_min, c);
    c_max = max(c_max, c);
}

tuple<int, vector<int>, vector<int>, vector<int>>
find_best(int r1, int r2, int c1, int c2, int dir){
    int N = (r2-r1+1) * (c2-c1+1);
    int dir2 = (dir+1) % 2;
    vector<int> R, C;
    vector<int> adj(N);
    for(int r=r1; r<=r2; r++){
    for(int c=c1; c<=c2; c++){
        R.push_back(r);
        C.push_back(c);
    }}
    for(int i=0;i<N;i++){
        adj[i] = T[R[i]][C[i]];
    }

    vector<int> x(N, 0);
    vector<int> best_t;
    int best_score = INT_MAX;
    while(true){
        vector<int> t(N);
        
        bool good = true;
        for(int i=0;i<N;i++){
            t[i] = zz[ adj[i] ][ dir ][ x[i] ];
            if(used[t[i]]) good = false;
        }
                
        if(good){
            int score=0;
            for(int i=0;i<N;i++){
                score += es[ adj[i] ][ t[i] ][ dir ];
                score += penalty[ adj[i] ][ dir ];
            }

            for(int i=0;i<N-1;i++){
                score += es[ t[i] ][ t[i+1] ][ dir2 ];
                score += penalty[ t[i] ][ dir2 ];
            }
            if(score <= best_score){
                for(int i=0;i<N;i++){
                    for(int j=i+1;j<N;j++)
                        if(t[i] == t[j])
                            good = false;
                }
                if(good){
                    best_t = t;
                    best_score = score;
                }
            }
        }

        int it = x.size()-1;
        while(it != -1 && x[it] == 20) it--;
        if(it == -1) break;
        x[it]++;
        while(++it < x.size()) x[it]=0;
    }

    return {best_score, R, C, best_t};    
}


void strat2(const array<array<array<uint8_t,3>,512>,512> &pixel, const vector<vector<vector<int>>> &es, const vector<vector<vector<int>>> &zz){ 
    penalty = vector<vector<int>>(N, vector<int>(4,0));
    for(int t=0;t<N;t++){
        for(int k=0;k<4;k++){
            penalty[t][k] = 2500/LM - min(es[t][zz[t][k][1]][k] - es[t][zz[t][k][0]][k], 2500/LM);
        }
    }

    // [1][2]
    // [3][4]
    // [5][6]
    int bestScore = INT_MAX;
    tuple<int,int,int,int,int,int> t_best;
    for(int t1=0;t1<N;t1++){
        for(int i=0;i<10;i++){
            int t2 = zz[t1][0][i]; if(t2 == t1) continue;
            for(int j=0;j<10;j++){
                int t3 = zz[t1][1][j]; if(t3 == t2 || t3 == t1) continue;
                for(int k = 0; k < 20; k++){
                    int t4 = (k < 10 ? zz[t3][0][k] : zz[t2][1][k-10]); if(t4 == t3 || t4 == t2 || t4 == t1) continue;
                    int scorep = es[t1][t2][0] +  es[t3][t4][0] + es[t1][t3][1] + es[t2][t4][1] + penalty[t1][0] + penalty[t3][0] + penalty[t1][1] + penalty[t2][1];
                    if(scorep > bestScore) continue;
                    for(int l = 0; l < 10; l++){
                        int t5 = zz[t3][1][l]; if(t5 == t4 || t5 == t3 || t5 == t2 || t5 == t1) continue;
                        for(int m = 0; m < 10; m++){
                            int t6 = zz[t4][1][m]; if(t6 == t5 || t6 == t4 || t6 == t3 || t6 == t2 || t6 == t1) continue;
                            int score = scorep + es[t5][t6][0] + es[t3][t5][1] + es[t4][t6][1] + penalty[t3][1] + penalty[t4][1] + penalty[t5][0];
                            if(score < bestScore){
                                t_best = {t1,t2,t3,t4,t5,t6};     
                                bestScore = score; 
                            }                               
                             
                        }
                    }    
                }
            }
        }
    }
    auto [t1,t2,t3,t4,t5,t6] = t_best;
    cout << bestScore << "\t" << t1 << " " << t2 << " " << t3 << " " << t4 << " " << t5 << " " << t6  << endl;
    place(t1, M, M);
    place(t2, M, M + 1);
    place(t3, M + 1, M);
    place(t4, M + 1, M + 1);
    place(t5, M + 2, M);
    place(t6, M + 2, M + 1);

    // while (height != 6 || width != 6)
    for(int iter=1;;iter++){
        int height = r_max - r_min + 1;
        int width =  c_max - c_min + 1;
        cout << height << " rows, " << width << " cols" << endl;
        if(height == M || width == M) break;
        
        bool expandV, expandH;
        if(height < 6 || width < 6){
            expandH = (width <= height);
            expandV = (height <= width);
        }
        else {
            expandH = (height == 6);
            expandV = (width == 6);
        }

        int bestScore = INT_MAX, best_dir;
        vector<int> bestR, bestC, best_t;
        if(expandH) {
            {   // right
                auto [score, R, C, t] = find_best(r_min, r_max, c_max, c_max, 0);
                // cout << score << " " << t.size() << endl;
                if(score <= bestScore){ bestScore = score; best_t = t; bestR = R; bestC = C; best_dir = 0; }   
            }   
            {   // left
                auto [score, R, C, t] = find_best(r_min, r_max, c_min, c_min, 2);
                // cout << score << " " << t.size() << endl;
                if(score <= bestScore){ bestScore = score; best_t = t; bestR = R; bestC = C; best_dir = 2; }  
            }
        }
        if(expandV){
            { // down
                auto [score, R, C, t] = find_best(r_max, r_max, c_min, c_max, 1);
                // cout << score << " " << t.size() << endl;
                if(score <= bestScore){ bestScore = score; best_t = t; bestR = R; bestC = C; best_dir = 1; }  
            }        
            {   // up
                auto [score, R, C, t] = find_best(r_min, r_min, c_min, c_max, 3);
                // cout << score << " " << t.size() << endl;
                if(score <= bestScore){ bestScore = score; best_t = t; bestR = R; bestC = C; best_dir = 3; }  
            }
        }
        cout << bestScore << "\t";
        for(auto t : best_t)
            cout << t << " ";
        cout << endl;
        
        for(int i=0;i<best_t.size();i++){
            place(best_t[i], bestR[i] + dirR[best_dir], bestC[i] + dirC[best_dir]);
        }        
    }

    for(int r=0;r<4*M;r++){
        for(int c=0;c<4*M;c++){
            if(T[r][c] != -1)
                fout << r << "\t" << c << "\t" << T[r][c] << endl;
        }
    }
}


int main(){
    cout << "Reading PNG '" << file_name << "'" << endl;
    read_png_file(file_name);
    cout << "Done Reading PNG" << endl;
    process_file();
    // cout << +pixel[511][510][0] << endl;
    // cout << +pixel[511][510][1] << endl;
    // cout << +pixel[511][510][2] << endl;

    compute_es(); // edge_score

    int total=0, cor=0, non=0;
    // int cor0=0, cor1=0,cor2=0,cor3=0;
    double totalAvgScore = 0, corAvgScore = 0, nonAvgScore = 0;
    double totalDist12 = 0, corDist12 = 0, incorDist12 = 0, nonDist12 = 0;
    double totalRatio12 = 0, corRatio12 = 0, incorRatio12 = 0, nonRatio12 = 0;
    int totalMaxScore=0, totalMinScore=INT_MAX, corMaxScore=0, corMinScore=INT_MAX, nonMaxScore=0, nonMinScore=INT_MAX, incorMaxScore=0, incorMinScore=INT_MAX;
    int totalMaxDist12=0, totalMinDist12=INT_MAX, corMaxDist12=0, corMinDist12=INT_MAX, nonMaxDist12=0, nonMinDist12=INT_MAX, incorMaxDist12=0, incorMinDist12=INT_MAX;
    double totalMaxRatio12=0, totalMinRatio12=INT_MAX, corMaxRatio12=0, corMinRatio12=INT_MAX, nonMaxRatio12=0, nonMinRatio12=INT_MAX, incorMaxRatio12=0, incorMinRatio12=INT_MAX;


    zz = vector<vector<vector<int>>>(N, vector<vector<int>>(4, vector<int>(N, 0)));
    for(int test = 0; test < N; test++){
        int test_r = test/M;
        int test_c = test%M;

        for(int k=0;k<4;k++){
            int actual_c = test_c, actual_r = test_r;
            bool exists_answer = false; // if a piece is on the edge, there will be no piece in that direction
            if(k == 0 && test_c + 1 != M){
                exists_answer = true;
                actual_c++;
            }
            if(k == 1 && test_r + 1 != M){
                exists_answer = true;
                actual_r++;
            }
            if(k == 2 && test_c != 0){
                exists_answer = true;
                actual_c--; 
            }
            if(k == 3 && test_r != 0){
                exists_answer = true;
                actual_r--;
            }
            int actual = M*actual_r + actual_c;
            // can we sort all potentials here instead of just picking the best?
            iota(zz[test][k].begin(), zz[test][k].end(), 0);
            sort(zz[test][k].begin(), zz[test][k].end(), [test, k](int i, int j){return es[test][i][k] < es[test][j][k];});
            
            vector<int> z = zz[test][k]; // copy
            // int bestScore;
            // int bestIndex = -1;
            // for(int a=0;a<N;a++){
            //     if(bestIndex == -1 || es[test][a][k] < bestScore){
            //         bestScore = es[test][a][k];
            //         bestIndex = a;
            //     }
            // }
            int bestIndex = z[0];
            int bestScore = es[test][z[0]][k];
            int ans_r = bestIndex/M;
            int ans_c = bestIndex%M;

            bool correct = false;
            if(exists_answer){
                total++; totalAvgScore += bestScore; totalMinScore = min(totalMinScore, bestScore); totalMaxScore = max(totalMaxScore, bestScore); 
                totalDist12 += es[test][z[1]][k] - es[test][z[0]][k];
                totalMinDist12 = min(totalMinDist12, es[test][z[1]][k] - es[test][z[0]][k]); totalMaxDist12 = max(totalMaxDist12, es[test][z[1]][k] - es[test][z[0]][k]);
                totalRatio12 += es[test][z[1]][k]/max(1.0,1.0+(double)es[test][z[0]][k]);
                if(bestIndex == actual){
                    cor++; corAvgScore += bestScore; corMinScore = min(corMinScore, bestScore); corMaxScore = max(corMaxScore, bestScore); 
                    corDist12 += es[test][z[1]][k] - es[test][z[0]][k];
                    corMinDist12 = min(corMinDist12, es[test][z[1]][k] - es[test][z[0]][k]); corMaxDist12 = max(corMaxDist12, es[test][z[1]][k] - es[test][z[0]][k]);
                    corRatio12 += es[test][z[1]][k]/max(1.0,1.0+(double)es[test][z[0]][k]);

                }
                else {
                    double actualScore = es[test][actual][k];                    
                    incorDist12 += es[test][z[1]][k] - es[test][z[0]][k];
                    incorMinDist12 = min(incorMinDist12, es[test][z[1]][k] - es[test][z[0]][k]); incorMaxDist12 = max(incorMaxDist12, es[test][z[1]][k] - es[test][z[0]][k]);
                    incorRatio12 += es[test][z[1]][k]/max(1.0,1.0+(double)es[test][z[0]][k]);
                    incorMinScore = min(incorMinScore, bestScore); incorMaxScore = max(incorMaxScore, bestScore); 
                }
            }
            else {
                non++; nonAvgScore += bestScore; nonMinScore = min(nonMinScore, bestScore); nonMaxScore = max(nonMaxScore, bestScore); 
                nonDist12 += es[test][z[1]][k] - es[test][z[0]][k];
                nonMinDist12 = min(nonMinDist12, es[test][z[1]][k] - es[test][z[0]][k]); nonMaxDist12 = max(nonMaxDist12, es[test][z[1]][k] - es[test][z[0]][k]);
                nonRatio12 += es[test][z[1]][k]/max(1.0,1.0+(double)es[test][z[0]][k]);
            }
        }
    }
    totalAvgScore /= total;
    corAvgScore /= cor;
    nonAvgScore /= non;

    corDist12 /= cor;
    totalDist12 /= total;
    incorDist12 /= (total - cor);
    nonDist12 /= non;

    corRatio12 /= cor;
    totalRatio12 /= total;
    incorRatio12 /= (total - cor);
    nonRatio12 /= non;

    cout << "total:\t" << total << endl;
    // cout << "cor0:\t" << cor0 << endl;
    // cout << "cor1:\t" << cor1 << endl;
    // cout << "cor2:\t" << cor2 << endl;
    // cout << "cor3:\t" << cor3 << endl;
    cout << "cor:\t" << cor << endl;
    cout << "corAvgScore:\t" << corAvgScore << "\t\t" << corMinScore << " - " << corMaxScore << endl;
    cout << "totalAvgScore:\t" << totalAvgScore << "\t\t" << totalMinScore << " - " << totalMaxScore << endl;
    double incorAvgScore = (totalAvgScore*total - corAvgScore*cor)/(total - cor);
    cout << "incorAvgScore:\t" << incorAvgScore << "\t\t" << incorMinScore << " - " << incorMaxScore << endl;
    cout << "nonAvgScore:\t" << nonAvgScore << "\t\t" << nonMinScore << " - " << nonMaxScore << endl;
    cout << endl;
    cout << "totalDist12:\t" << totalDist12 << "\t\t" << totalMinDist12 << " - " << totalMaxDist12 << endl;
    cout << "corDist12:\t" << corDist12 << "\t\t" << corMinDist12 << " - " << corMaxDist12 << endl;
    cout << "incorDist12:\t" << incorDist12 << "\t\t" << incorMinDist12 << " - " << incorMaxDist12 << endl;
    cout << "nonDist12:\t" << nonDist12 << "\t\t" << nonMinDist12 << " - " << nonMaxDist12 << endl;
    cout << endl;
    cout << "totalRatio12:\t" << totalRatio12 << endl;
    cout << "corRatio12:\t" << corRatio12 << endl;
    cout << "incorRatio12:\t" << incorRatio12 << endl;
    cout << "nonRatio12:\t" << nonRatio12 << endl;
    cout << endl;

    // test
    // for each piece, which piece is found in some direction?
    // constraint: we may use each edge of a piece only once, eg it may only be a 'top' once
    // but no further geometrical constraints are applied for now
    vector<vector<int>> actual(N, vector<int>(4, -1));
    for(int test = 0; test < N; test++){
        int test_r = test/M;
        int test_c = test%M;
        if(test_c + 1 != M)
            actual[test][0] = (test_r)*M  + (test_c + 1);
        if(test_r + 1 != M)
            actual[test][1] = (test_r + 1)*M  + (test_c);
        if(test_c != 0)
            actual[test][2] = (test_r)*M  + (test_c - 1);
        if(test_r != 0)
            actual[test][3] = (test_r - 1)*M  + (test_c);
    }
    

    strat2(pixel, es, zz);

    
    

/*    
    vector<vector<int>> res(N, vector<int>(4, -1));
    vector<vector<bool>> used(N, vector<bool>(4, false));
    vector<vector<int>> iter_1(N, vector<int>(4, 0));
    vector<vector<int>> iter_2(N, vector<int>(4, 1));

    vector<int> dir_count(2, 0);

    int correct1 = 0;
    for(int iter=1;iter<=2*M*(M-1);iter++){
        int bestT=-1, bestK;
        int bestScore;
        for(int t=0;t<N;t++){
        for(int k=0;k<4;k++){
            if(dir_count[k] == M*(M-1)) continue;
            int _k = (k+2)%4;
            if(used[t][k]) continue;
            while(iter_1[t][k] != N && (used[zz[t][k][iter_1[t][k]]][_k] || zz[t][k][iter_1[t][k]] == t)) iter_1[t][k]++;
            while(iter_2[t][k] != N && (used[zz[t][k][iter_2[t][k]]][_k] || zz[t][k][iter_2[t][k]] == t || !(iter_1[t][k] < iter_2[t][k]))) iter_2[t][k]++;
            if(iter_1[t][k] == N || iter_2[t][k] == N){
                cout << dir_count[0] << " " << dir_count[1] << endl;
                cout << iter_1[t][k] << " " << iter_2[t][k] << endl;
                cout << "?!" << endl;
                exit(0);
            }

            // cout << t << "\t" << k;
            // cout << "\t" << es[t][zz[t][k][iter_1[t][k]]][k] << "\t" << es[t][zz[t][k][iter_2[t][k]]][k] << "\t";
            // cout << (zz[t][k][iter_1[t][k]] == actual[t][k]) << endl;

            int s1 = es[t][zz[t][k][iter_1[t][k]]][k];
            int s2 = es[t][zz[t][k][iter_2[t][k]]][k];
            int score = max(2500 - (s2 - s1), 0) + 3*s1; // different images may work better/worse with lower coefficient weights here
            if(bestT == -1 || score <= bestScore){
                bestScore = score;
                bestT = t;
                bestK = k;
            }
        }}
        int _bestK = (bestK+2)%4;
        // cout << bestScore << "\t" << bestT << "," << bestK << "\t" << (zz[bestT][bestK][iter_1[bestT][bestK]] == actual[bestT][bestK]) << endl;
        int ans = zz[bestT][bestK][iter_1[bestT][bestK]];
        res[bestT][bestK] = ans;
        used[bestT][bestK] = true;
        used[ans][_bestK] = true;
        dir_count[bestK%2]++;

        correct1 += (res[bestT][bestK] == actual[bestT][bestK]);
    }
    cout << "correct1:" << correct1 << endl;
*/


/*
    // start filling guesses into res[i][k]
    // idea: want edge that is indisputably the best fit, ie biggest difference from 2nd best
    // TEST ALGO START
    vector<int> order(4*N); iota(order.begin(), order.end(), 0);
    cout << "sort start" << endl;
    sort(order.begin(), order.end(), [&es, &zz](int i,int j){
        const int it = i/4; const int jt = j/4;
        const int ik = i%4; const int jk = j%4;
        // return 1*es[it][(zz[it][ik][1])][ik] - 3*es[it][(zz[it][ik][0])][ik] > 1*es[jt][(zz[jt][jk][1])][jk] - 3*es[jt][(zz[jt][jk][0])][jk];
        const int i1 = es[it][(zz[it][ik][0])][ik];
        const int i2 = es[it][(zz[it][ik][1])][ik];
        const int j1 = es[jt][(zz[jt][jk][0])][jk];
        const int j2 = es[jt][(zz[jt][jk][1])][jk];
        return max(5000 - (i2 - i1), 0) + 2*i1 < max(5000 - (j2 - j1), 0) + 2*j1;
    });
    cout << "sort complete" << endl;
    // for(int t = 0; t < N; t++){
    //     for(int k = 0; k < 4; k++){
    //         int p1 = zz[t][k][0];
    //         int p2 = zz[t][k][1];
    //         // cout << t << "," << k << "\t" << es[t][p2][k]-es[t][p1][k] << endl;
    //     }
    // }
    int run = 0;
    int incorrect = 0;
    for(auto x : order){
        int k = x%4;
        int t = x/4;
        // cout << x << "," << k << "\t" << es[t][zz[t][k][1]][k] - es[t][zz[t][k][0]][k] << "\t" << es[t][zz[t][k][0]][k] << endl;
        // cout << "\t" << zz[t][k][0] << " " << actual[t][k] << endl;
        if(zz[t][k][0] != actual[t][k]) incorrect++;
        if(incorrect == 1) break;
        run++;
        fout << t << " " << k << endl;
    }
    cout << "correct in a row:" << run << endl;
*/
    // TODO:
    // for each t,k: keep track of the best 2 options that are still available, ie this edge hasn't been used already
    // pick the pair (t,k) which minimises then function "max(2500 - (s2 - s1), 0) + 3*s1
    // until: ??




    // ALTERNATIVELY: 
    // actually place the pieces down when we make an edge
}


