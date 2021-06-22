#define DLLEXPORT extern "C" __declspec(dllexport)
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <chrono>
#include <ctime>
#include <sstream>
using namespace std;

const char* nice_print(std::vector<std::pair<int, int>> arc_permutation_encoding){
    std::stringstream buffer;
    buffer <<"[[";
    for(int i=0; i<arc_permutation_encoding.size();i++){
        buffer<<arc_permutation_encoding[i].first << ",  ";
        }
        buffer<<"],\n";
    buffer <<"[";
    for(int i=0; i<arc_permutation_encoding.size();i++){
        buffer<<arc_permutation_encoding[i].second << ",  ";
        }
    buffer<<"]]";
    return buffer.str().c_str();
    }


bool is_embedded(std::pair<int, int> edge1, std::pair<int, int> edge2)
{
    int min1 = std::min(edge1.first, edge1.second);
    int max1 = std::max(edge1.first, edge1.second);
    int min2 = std::min(edge2.first, edge2.second);
    int max2 = std::max(edge2.first, edge2.second);
    if(min1>=min2 && max1<=max2)
    {
        return 1;
    } else if(min2>=min1 && max2<=max1){
        return 1;
        } else {
            return 0;
            }
}

bool is_intersecting(std::pair<int, int> edge1, std::pair<int, int> edge2)
{
    int min1 = std::min(edge1.first, edge1.second);
    int max1 = std::max(edge1.first, edge1.second);
    int min2 = std::min(edge2.first, edge2.second);
    int max2 = std::max(edge2.first, edge2.second);
    return !(max1<=min2 or max2<=min1);
}

bool is_interleaving(std::pair<int, int> edge1, std::pair<int, int> edge2)
{
    return is_intersecting(edge1, edge2) && !is_embedded(edge1, edge2);
}

std::vector<std::pair<int, int>> stabilize(std::vector<std::pair<int, int>> arc_permutation_encoding, int edge_index){
    //edge_index = edge_index%arc_permutation_encoding.size();
    //int prev_edge_index = (edge_index-1+arc_permutation_encoding.size())%arc_permutation_encoding.size();
    int prev_edge_index = edge_index-1;
    std::pair<int, int> edge = arc_permutation_encoding[prev_edge_index];
    int x1 = edge.first;
    int x2 = edge.second;
    for(int i=0; i<arc_permutation_encoding.size();i++){
        if(arc_permutation_encoding[i].first >=x1){
            arc_permutation_encoding[i].first += 1;
            }
        if(arc_permutation_encoding[i].second >= x1){
            arc_permutation_encoding[i].second += 1;
            }
        }
    std::pair<int, int> inserted_edge = std::make_pair(x1+1, x1);
    arc_permutation_encoding.insert(arc_permutation_encoding.begin()+prev_edge_index, inserted_edge);
    arc_permutation_encoding[edge_index].first-=1;
    //std::cout << nice_print(arc_permutation_encoding)<<std::endl;
    return arc_permutation_encoding;
}

std::vector<std::pair<int, int>> get_dual_encoding(std::vector<std::pair<int, int>> arc_permutation_encoding){
    std::vector<std::pair<int, int>> mid1;
    std::vector<std::pair<int, int>> mid2;
    for(int i=0; i<arc_permutation_encoding.size();i++){
        mid1.push_back(std::make_pair(arc_permutation_encoding[i].first,i+1));
        mid2.push_back(std::make_pair(arc_permutation_encoding[i].second,i+1));
        }
    std::sort(mid1.begin(), mid1.end());
    std::sort(mid2.begin(), mid2.end());
    std::vector<std::pair<int, int>> result;
    for(int i=0; i<arc_permutation_encoding.size();i++){
        result.push_back(std::make_pair(mid1[i].second, mid2[i].second));
        }
    return result;
    }

std::vector<std::pair<int, int>> switch_edges(std::vector<std::pair<int, int>> arc_permutation_encoding, int ind1, int ind2, bool vertical)
{
    //external switch enabling
    int diagsize = arc_permutation_encoding.size();
    ind1 = (ind1-1+diagsize)%diagsize;
    ind2 = (ind2-1+diagsize)%diagsize;
    
    if(vertical){
        std::vector<std::pair<int, int>> dual_encoding = get_dual_encoding(arc_permutation_encoding);
        std::pair<int, int> tmp = std::make_pair(dual_encoding[ind1].first, dual_encoding[ind1].second);
        dual_encoding[ind1] = dual_encoding[ind2];
        dual_encoding[ind2] = tmp;
        std::vector<std::pair<int, int>> res = get_dual_encoding(dual_encoding);
        return res; 
    }else{
        std::pair<int, int> tmp = std::make_pair(arc_permutation_encoding[ind1].first, arc_permutation_encoding[ind1].second);
        arc_permutation_encoding[ind1] = arc_permutation_encoding[ind2];
        arc_permutation_encoding[ind2] = tmp;
        return arc_permutation_encoding;
    }
}

std::vector<std::pair<int, int>> translate(std::vector<std::pair<int, int>> arc_permutation_encoding, bool vertical, bool sign)
{
    if(vertical){
        std::vector<std::pair<int, int>> dual_encoding = get_dual_encoding(arc_permutation_encoding);
        std::vector<std::pair<int, int>> tmp(dual_encoding.size());
            if(sign){
                std::rotate_copy(dual_encoding.begin(), dual_encoding.begin()+1, dual_encoding.end(), tmp.begin());
            }else{
                std::rotate_copy(dual_encoding.begin(), dual_encoding.end()-1, dual_encoding.end(), tmp.begin());
            }
        std::vector<std::pair<int, int>> res = get_dual_encoding(tmp);
        return res; 
    }else{
        std::vector<std::pair<int, int>> res(arc_permutation_encoding.size());
        if(sign){
                std::rotate_copy(arc_permutation_encoding.begin(), arc_permutation_encoding.begin()+1, arc_permutation_encoding.end(), res.begin());
            }else{
                std::rotate_copy(arc_permutation_encoding.begin(), arc_permutation_encoding.end()-1, arc_permutation_encoding.end(), res.begin());
            }
        return res;
    }
}




extern "C" {
    
    auto int_distr = std::uniform_int_distribution<>(0,1);
    auto rand_engine = std::default_random_engine();
    auto gen = std::bind(int_distr, rand_engine);

    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    
    std::uniform_int_distribution<int> uni_rows(0, 100);
    
    std::vector<std::pair<int, int>> entangle(std::vector<std::pair<int, int>>enc, int max_complexity, int switch_max){ 
    
    bool vertical = 0;
    vertical = gen();
    bool validity_flag = 0;
    int index=1;
    std::uniform_int_distribution<int> stab_rows(1,enc.size()-1);
    while(enc.size()<max_complexity){
            index = stab_rows(rng);
            //std::cout<<index<<std::endl;
            //index = index%enc.size();
            enc = stabilize(enc, index);
            stab_rows = std::uniform_int_distribution<int>(1,enc.size()-1);
    } 
    //uni_rows = std::uniform_int_distribution<int>(2, enc.size()-2);
    for(int tick=0; tick<switch_max; tick++){
        std::vector<std::pair<int, int>> dual_enc = get_dual_encoding(enc);
        validity_flag = 0;
        while(!validity_flag){
            index = uni_rows(rng);
            //if (index==enc.size()){
            //    std::cout<<"external switch!"<<std::endl;
            //}
            index = index%enc.size();
            vertical = gen();
            if(vertical){
                validity_flag = !is_interleaving(dual_enc[index-1], dual_enc[index]);
            }else{
                validity_flag = !is_interleaving(enc[index-1], enc[index]);
            }
        }
        enc = switch_edges(enc, index, index+1, vertical);
        //translation moves addition
        bool iftranslate = gen();
        bool trvert = gen();
        bool trleft = gen();
        //if(iftranslate){
        //    enc = translate(enc, trvert, trleft);
        //}
    }

    return enc;
}
}



std::vector<std::pair<int, int>>construct_diagram(std::vector<int> row1, std::vector<int> row2)
{
    //assert row1.size()==row2.size();
    std::vector<std::pair<int, int>> result;
    for(int i=0; i<row1.size();i++){
        result.push_back(std::make_pair(row1[i], row2[i]));
        }
    return result;
}

//definition of them knots
//trivial
std::pair<int, int> triv_edge_1 = std::make_pair(1,2);
std::pair<int, int> triv_edge_2 = std::make_pair(2,1);
std::vector<std::pair<int, int>> trivial_knot{triv_edge_1, triv_edge_2};

//3_1(a.k.a trefoil)
std::vector<int> top_3_1{2,1,5,4,3};
std::vector<int> bottom_3_1{5,4,3,2,1};
std::vector<std::pair<int, int>> enc_3_1 = construct_diagram(top_3_1, bottom_3_1);

//4_1
std::vector<int> top_4_1{3,2,6,4,5,7,1};
std::vector<int> bottom_4_1{7,5,3,2,1,4,6};
std::vector<std::pair<int, int>> enc_4_1 = construct_diagram(top_4_1, bottom_4_1);

//5_1
std::vector<int> top_5_1{2,1,7,6,5,4,3};
std::vector<int> bottom_5_1{7,6,5,4,3,2,1};
std::vector<std::pair<int, int>> enc_5_1 = construct_diagram(top_5_1, bottom_5_1);

//5_2
std::vector<int> top_5_2{3,2,9,5,8,7,1,6,4};
std::vector<int> bottom_5_2{9,8,7,3,4,6,5,2,1};
std::vector<std::pair<int, int>> enc_5_2 = construct_diagram(top_5_2, bottom_5_2);

//6_1
std::vector<int> top_6_1{4,3,8,11,10,2,7,5,6,9,1};
std::vector<int> bottom_6_1{11,10,4,6,9,8,3,2,1,5,7};
std::vector<std::pair<int, int>> enc_6_1 = construct_diagram(top_6_1, bottom_6_1);

//6_2
std::vector<int> top_6_2{3,2,9,8,6,4,5,7,1};
std::vector<int> bottom_6_2{9,8,7,5,3,2,1,4,6};
std::vector<std::pair<int, int>> enc_6_2 = construct_diagram(top_6_2, bottom_6_2);

//6_3
std::vector<int> top_6_3{3,2,9,7,4,5,6,8,1};
std::vector<int> bottom_6_3{9,8,6,3,2,1,4,5,7};
std::vector<std::pair<int, int>> enc_6_3 = construct_diagram(top_6_3, bottom_6_3);

//7_1
std::vector<int> top_7_1{2,1,9,8,7,6,5,4,3};
std::vector<int> bottom_7_1{9,8,7,6,5,4,3,2,1};
std::vector<std::pair<int, int>> enc_7_1 = construct_diagram(top_7_1, bottom_7_1);

//7_2
std::vector<int> top_7_2{4,1,3,9,8,6,2,7,5};
std::vector<int> bottom_7_2{9,7,6,2,5,1,8,4,3};
std::vector<std::pair<int, int>> enc_7_2 = construct_diagram(top_7_2, bottom_7_2);

//7_3
std::vector<int> top_7_3{9,1,8,6,7,5,4,3,2};
std::vector<int> bottom_7_3{6,7,5,4,9,3,2,1,8};
std::vector<std::pair<int, int>> enc_7_3 = construct_diagram(top_7_3, bottom_7_3);

//7_4
std::vector<int> top_7_4{3,1,2,7,9,8,6,4,5};
std::vector<int> bottom_7_4{9,6,8,5,4,1,3,7,2};
std::vector<std::pair<int, int>> enc_7_4 = construct_diagram(top_7_4, bottom_7_4);

//7_5
std::vector<int> top_7_5{1,9,8,2,7,5,4,6,3};
std::vector<int> bottom_7_5{8,7,5,6,4,3,2,1,9};
std::vector<std::pair<int, int>> enc_7_5 = construct_diagram(top_7_5, bottom_7_5);

//7_6
std::vector<int> top_7_6{2,8,6,4,1,9,5,7,3};
std::vector<int> bottom_7_6{6,5,9,8,7,3,2,4,1};
std::vector<std::pair<int, int>> enc_7_6 = construct_diagram(top_7_6, bottom_7_6);

//7_7
std::vector<int> top_7_7{7,5,8,6,3,1,2,4,9};
std::vector<int> bottom_7_7{3,9,4,2,8,5,7,1,6};
std::vector<std::pair<int, int>> enc_7_7 = construct_diagram(top_7_7, bottom_7_7);

//8_1
std::vector<int> top_8_1{5,4,12,15,14,3,9,13,11,2,8,6,7,10,1};
std::vector<int> bottom_8_1{15,14,5,11,13,12,4,7,10,9,3,2,1,6,8};
std::vector<std::pair<int, int>> enc_8_1 = construct_diagram(top_8_1, bottom_8_1);

//8_2
std::vector<int> top_8_2{3,2,11,10,9,8,6,4,5,7,1};
std::vector<int> bottom_8_2{11,10,9,8,7,5,3,2,1,4,6};
std::vector<std::pair<int, int>> enc_8_2 = construct_diagram(top_8_2, bottom_8_2);

//8_3
std::vector<int> top_8_3{5,4,12,15,14,3,11,9,8,6,7,10,13,1,2}; 
std::vector<int> bottom_8_3{15,14,5,10,13,12,4,7,3,1,2,6,9,11,8};
std::vector<std::pair<int, int>> enc_8_3 = construct_diagram(top_8_3, bottom_8_3);

//8_4
std::vector<int> top_8_4{4,3,13,12,10,8,7,5,6,9,11,1,2};  
std::vector<int> bottom_8_4{13,12,11,9,4,6,3,1,2,5,8,10,7};
std::vector<std::pair<int, int>> enc_8_4 = construct_diagram(top_8_4, bottom_8_4);

//8_5
std::vector<int> top_8_5{3,2,11,10,8,7,6,4,5,9,1};   
std::vector<int> bottom_8_5{11,10,9,7,6,5,3,2,1,4,8};
std::vector<std::pair<int, int>> enc_8_5 = construct_diagram(top_8_5, bottom_8_5);

//8_6
std::vector<int> top_8_6{4,3,13,12,8,11,10,2,7,5,6,9,1};   
std::vector<int> bottom_8_6{13,12,11,10,4,6,9,8,3,2,1,5,7};
std::vector<std::pair<int, int>> enc_8_6 = construct_diagram(top_8_6, bottom_8_6);

//8_7
std::vector<int> top_8_7{3,2,11,10,9,7,4,5,6,8,1};   
std::vector<int> bottom_8_7{11,10,9,8,6,3,2,1,4,5,7};
std::vector<std::pair<int, int>> enc_8_7 = construct_diagram(top_8_7, bottom_8_7);

//8_8
std::vector<int> top_8_8{4,3,13,9,12,11,2,8,5,6,7,10,1};
std::vector<int> bottom_8_8{13,12,11,4,7,10,9,3,2,1,5,6,8};
std::vector<std::pair<int, int>> enc_8_8 = construct_diagram(top_8_8, bottom_8_8);

//8_9
std::vector<int> top_8_9{3,2,11,10,8,4,5,6,7,9,1};
std::vector<int> bottom_8_9{11,10,9,7,3,2,1,4,5,6,8};
std::vector<std::pair<int, int>> enc_8_9 = construct_diagram(top_8_9, bottom_8_9);

//8_10
std::vector<int> top_8_10{3,2,11,10,8,7,4,5,6,9,1};
std::vector<int> bottom_8_10{11,10,9,7,6,3,2,1,4,5,8};
std::vector<std::pair<int, int>> enc_8_10 = construct_diagram(top_8_10, bottom_8_10);

//8_11
std::vector<int> top_8_11{4,3,10,13,12,2,11,9,7,5,6,8,1};
std::vector<int> bottom_8_11{13,12,4,9,11,10,8,6,3,2,1,5,7};
std::vector<std::pair<int, int>> enc_8_11 = construct_diagram(top_8_11, bottom_8_11);

//8_12
std::vector<int> top_8_12{5,4,12,9,11,13,3,2,8,6,7,10,1};
std::vector<int> bottom_8_12{13,11,5,4,7,10,12,9,3,2,1,6,8};
std::vector<std::pair<int, int>> enc_8_12 = construct_diagram(top_8_12, bottom_8_12);

//8_13
std::vector<int> top_8_13{4,3,13,11,8,7,5,6,9,10,12,1,2};
std::vector<int> bottom_8_13{13,12,10,4,6,3,1,2,5,8,9,11,7};
std::vector<std::pair<int, int>> enc_8_13 = construct_diagram(top_8_13, bottom_8_13);

//8_14
std::vector<int> top_8_14{4,3,13,9,12,11,2,10,7,5,6,8,1};
std::vector<int> bottom_8_14{13,12,11,4,8,10,9,6,3,2,1,5,7};
std::vector<std::pair<int, int>> enc_8_14 = construct_diagram(top_8_14, bottom_8_14);

//8_15
std::vector<int> top_8_15{4,3,13,11,10,12,2,1,8,7,6,9,5};
std::vector<int> bottom_8_15{13,12,10,4,8,9,11,7,6,3,5,2,1};
std::vector<std::pair<int, int>> enc_8_15 = construct_diagram(top_8_15, bottom_8_15);

//8_16
std::vector<int> top_8_16{3,2,11,9,8,6,4,5,7,10,1};
std::vector<int> bottom_8_16{11,10,8,7,5,3,2,1,4,6,9};
std::vector<std::pair<int, int>> enc_8_16 = construct_diagram(top_8_16, bottom_8_16);

//8_17
std::vector<int> top_8_17{3,2,11,9,7,4,5,6,8,10,1};
std::vector<int> bottom_8_17{11,10,8,6,3,2,1,4,5,7,9};
std::vector<std::pair<int, int>> enc_8_17 = construct_diagram(top_8_17, bottom_8_17);

//8_18
std::vector<int> top_8_18{3,2,10,8,6,4,5,7,9,11,1};
std::vector<int> bottom_8_18{11,9,7,5,3,2,1,4,6,8,10};
std::vector<std::pair<int, int>> enc_8_18 = construct_diagram(top_8_18, bottom_8_18);

//8_19
std::vector<int> top_8_19{3,2,11,10,9,1,7,6,5,8,4};
std::vector<int> bottom_8_19{11,10,9,7,8,6,5,3,4,2,1};
std::vector<std::pair<int, int>> enc_8_19 = construct_diagram(top_8_19, bottom_8_19);

//8_20
std::vector<int> top_8_20{3,2,11,5,4,6,7,10,8,9,1};
std::vector<int> bottom_8_20{11,10,9,3,2,1,5,6,7,4,8};
std::vector<std::pair<int, int>> enc_8_20 = construct_diagram(top_8_20, bottom_8_20);

//8_21
std::vector<int> top_8_21{3,2,11,6,7,10,9,1,8,5,4};
std::vector<int> bottom_8_21{11,10,9,3,5,6,8,7,4,2,1};
std::vector<std::pair<int, int>> enc_8_21 = construct_diagram(top_8_21, bottom_8_21);






std::vector<std::vector<std::pair<int, int>>> knots{trivial_knot, enc_3_1, enc_4_1, enc_5_1, enc_5_2, enc_6_1, enc_6_2, enc_6_3, enc_7_1, enc_7_2, enc_7_3, enc_7_4, enc_7_5, enc_7_6, enc_7_7, enc_8_1,enc_8_2,enc_8_3, enc_8_4, enc_8_5, enc_8_6, enc_8_7, enc_8_8, enc_8_9,
enc_8_10, enc_8_11, enc_8_12, enc_8_13, enc_8_14, enc_8_15, enc_8_16, enc_8_17, enc_8_18, enc_8_19, enc_8_20, enc_8_21};

//wrapper which generates complicated diagram given class label with dual encoding attached
extern "C" {
void complicate_wrapper(int label,  int max_complexity, int switch_max, int *bottomrow, int *toprow, int *toprow_dual, int *bottomrow_dual)
{
    std::vector<std::pair<int, int>> starting_diag = knots[label];
    std::vector<std::pair<int, int>> res = entangle(starting_diag, max_complexity, switch_max);
    std::vector<std::pair<int, int>> res_dual = get_dual_encoding(res);
    //std::vector<std::pair<int, int>> res_dual = get_dual_encoding(res_dual1);
    for(int i=0; i<max_complexity; i++){
        bottomrow[i]=res[i].first;
        toprow[i]=res[i].second;
        bottomrow_dual[i]=res_dual[i].first;
        toprow_dual[i]=res_dual[i].second;
    }

}
}



extern "C" {
void gradual_complicate_wrapper(int current_complexity, int max_complexity, int switch_max, int *bottomrow_in, int *toprow_in, int *bottomrow, int *toprow, int *toprow_dual, int *bottomrow_dual)
{
    //std::vector<int> v1(bottomrow_in, bottomrow_in + sizeof(int)*current_complexity);
    //std::vector<int> v2(toprow_in, toprow_in + sizeof(int)*current_complexity);
    std::vector<std::pair<int, int>> starting_diag;
    for(int i=0; i<current_complexity;i++){
        starting_diag.push_back(std::make_pair(toprow_in[i], bottomrow_in[i]));
        }
    
    //std::vector<std::pair<int, int>> starting_diag = construct_diagram(v2, v1);
    std::vector<std::pair<int, int>> res = entangle(starting_diag, max_complexity, switch_max);
    std::vector<std::pair<int, int>> res_dual = get_dual_encoding(res);
    //std::vector<std::pair<int, int>> res_dual = get_dual_encoding(res_dual1);
    for(int i=0; i<max_complexity; i++){
        bottomrow[i]=res[i].first;
        toprow[i]=res[i].second;
        //bottomrow[i]=v1[i];
        //toprow[i]=v2[i];
        bottomrow_dual[i]=res_dual[i].first;
        toprow_dual[i]=res_dual[i].second;
    }

}
}





int main()
{
//std::pair<int, int> edge_1 = std::make_pair(1,2);
//std::pair<int, int> edge_2 = std::make_pair(2,1);
//std::vector<std::pair<int, int>> trivial_knot{edge_1, edge_2};
//std::vector<int> vec1{3,2,9,7,4,5,6,8,1};
//std::vector<int> vec2{9,8,6,3,2,1,4,5,7};
//std::vector<int>vec1 {9, 2, 1, 5, 6, 7, 4, 8, 3};
//std::vector<int>vec2 {6, 5, 4, 7, 8, 3, 9, 2, 1};

//std::vector<std::pair<int, int>> knot = construct_diagram(vec1, vec2);
//std::vector<std::pair<int, int>> knotent = entangle(knot,25, 100000);
//std::vector<std::pair<int, int>> knotent = switch_edges(knot,9,10,0);
//std::vector<std::pair<int, int>> knotent = stabilize(knot, 8);
//std::cout << nice_print(knotent);
//for(int i=0; i<knotent.size();i++){
//    std:cout<<"["<<knotent[i].first<< ", "<<knotent[i].second<<"],"<<std::endl;
//} 

//std::cout << "finished computation at " << std::ctime(&end_time)
//              << "elapsed time: " << elapsed_seconds.count() << "s\n";
return 0;
}