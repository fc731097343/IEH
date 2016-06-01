#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <ctime>
#include "distance.h"

#define MAX_ROWSIZE 1024
#define HASH_RADIUS 1
#define DEPTH 16 //smaller than code length
#define INIT_NUM 5500
#define POOL_SIZE 1100
using namespace std;
typedef vector<vector<float> > Matrix;
typedef vector<unsigned int> Codes;
typedef unordered_map<unsigned int, vector<unsigned int> > HashBucket;
typedef vector<HashBucket> HashTable;
typedef std::set<Candidate<float>, std::greater<Candidate<float> > > CandidateHeap;

void StringSplit(string src, vector<string>& des){
  int start = 0;
  int end = 0;
  for(size_t i = 0; i < src.length(); i++){
    if(src[i]==' '){
      end = i;
      //if(end>start)cout<<start<<" "<<end<<" "<<src.substr(start,end-start)<<endl;
      des.push_back(src.substr(start,end-start));
      start = i+1;
    }
  }
}

void LoadBaseCode(char* filename, Codes& base){
  ifstream in(filename);
  char buf[MAX_ROWSIZE];
  //int cnt = 0;
  while(!in.eof()){
    in.getline(buf,MAX_ROWSIZE);
    string strtmp(buf);
    vector<string> strs;
    StringSplit(strtmp,strs);
    if(strs.size()<2)continue;
    unsigned int codetmp = 0;
    for(size_t i = 0; i < strs.size(); i++){
      unsigned int c = atoi(strs[i].c_str());
      codetmp = codetmp << 1;
      codetmp += c;

    }//if(cnt++ > 999998){cout<<strs.size()<<" "<<buf<<" "<<codetmp<<endl;}
    base.push_back(codetmp);
  }//cout<<base.size()<<endl;
  in.close();
}

void LoadHashFunc(char* filename, Matrix& func){
  ifstream in(filename);
  char buf[MAX_ROWSIZE];

  while(!in.eof()){
    in.getline(buf,MAX_ROWSIZE);
    string strtmp(buf);
    vector<string> strs;
    StringSplit(strtmp,strs);
    if(strs.size()<2)continue;
    vector<float> ftmp;
    for(size_t i = 0; i < strs.size(); i++){
      float f = atof(strs[i].c_str());
      ftmp.push_back(f);
      //cout<<f<<" ";
    }//cout<<endl;
    //cout<<strtmp<<endl;
    func.push_back(ftmp);
  }//cout<<func.size()<<endl;
  in.close();

}

void LoadData(char* filename, Matrix& dataset){
  ifstream in(filename, ios::binary);
  if(!in.is_open()){cout<<"open file error"<<endl;exit(-1);}
  unsigned int dim;
  in.read((char*)&dim,4);
  cout<<"data dimension: "<<dim<<endl;
  in.seekg(0,ios::end);
  ios::pos_type ss = in.tellg();
  size_t fsize = (size_t)ss;
  unsigned int num = fsize / (dim+1) / 4;
  in.seekg(0,ios::beg);
  for(size_t i = 0; i < num; i++){
    in.seekg(4,ios::cur);
    vector<float> vtmp(dim);
    vtmp.clear();
    for(size_t j = 0; j < dim; j++){
      float tmp;
      in.read((char*)&tmp,4);
      vtmp.push_back(tmp);
    }
    dataset.push_back(vtmp);
  }//cout<<dataset.size()<<endl;
  in.close();
}

void BuildHashTable(int upbits, int lowbits, Codes base ,HashTable& tb){
  tb.clear();
  for(int i = 0; i < (1 << upbits); i++){
    HashBucket emptyBucket;
    tb.push_back(emptyBucket);
  }
  for(size_t i = 0; i < base.size(); i ++){
    unsigned int idx1 = base[i] >> lowbits;
    unsigned int idx2 = base[i] - (idx1 << lowbits);
    if(tb[idx1].find(idx2) != tb[idx1].end()){
      tb[idx1][idx2].push_back(i);
    }else{
      vector<unsigned int> v;
      v.push_back(i);
      tb[idx1].insert(make_pair(idx2,v));
    }
  }
}

bool MatrixMultiply(Matrix A, Matrix B, Matrix& C){
  if(A.size() == 0 || B.size() == 0){cout<<"matrix a or b size 0"<<endl;return false;}
  else if(A[0].size() != B.size()){
    cout<<"--error: matrix a, b dimension not agree"<<endl;
    cout<<"A"<<A.size()<<" * "<<A[0].size()<<endl;
    cout<<"B"<<B.size()<<" * "<<B[0].size()<<endl;
    return false;
  }
  for(size_t i = 0; i < A.size(); i++){
    vector<float> tmp;
    for(size_t j = 0; j < B[0].size(); j++){
      float fnum = 0;
      for(size_t k=0; k < B.size(); k++)fnum += A[i][k] * B[k][j];
      tmp.push_back(fnum);
    }
    C.push_back(tmp);
  }
  return true;
}
void QueryToCode(Matrix query, Matrix func, Codes& querycode){
  Matrix Z;
  if(!MatrixMultiply(query, func, Z)){return;}
  for(size_t i = 0; i < Z.size(); i++){
    unsigned int codetmp = 0;
    for(size_t j = 0; j < Z[0].size(); j++){
      if(Z[i][j]>0){codetmp = codetmp << 1;codetmp += 1;}
      else {codetmp = codetmp << 1;codetmp += 0;}
    }
    //if(i<3)cout<<codetmp<<endl;
    querycode.push_back(codetmp);
  }//cout<<querycode.size()<<endl;
}

void HashTest(int upbits,int lowbits, Codes querycode, HashTable tb, vector<vector<int> >& cands){
  for(size_t i = 0; i < querycode.size(); i++){

    unsigned int idx1 = querycode[i] >> lowbits;
    unsigned int idx2 = querycode[i] - (idx1 << lowbits);
    HashBucket::iterator bucket= tb[idx1].find(idx2);
    vector<int> canstmp;
    if(bucket != tb[idx1].end()){
      vector<unsigned int> vp = bucket->second;
      //cout<<i<<":"<<vp.size()<<endl;
      for(size_t j = 0; j < vp.size() && canstmp.size() < INIT_NUM; j++){
        canstmp.push_back(vp[j]);
      }
    }


    if(HASH_RADIUS == 0){
      cands.push_back(canstmp);
      continue;
    }
    for(size_t j = 0; j < DEPTH; j++){
      unsigned int searchcode = querycode[i] ^ (1 << j);
      unsigned int idx1 = searchcode >> lowbits;
      unsigned int idx2 = searchcode - (idx1 << lowbits);
      HashBucket::iterator bucket= tb[idx1].find(idx2);
      if(bucket != tb[idx1].end()){
        vector<unsigned int> vp = bucket->second;
        for(size_t k = 0; k < vp.size() && canstmp.size() < INIT_NUM; k++){
          canstmp.push_back(vp[k]);
        }
      }
    }
    cands.push_back(canstmp);
  }
}

void LoadKnnTable(char* filename, vector<CandidateHeap >& tb){
  std::ifstream in(filename,std::ios::binary);
  in.seekg(0,std::ios::end);
  std::ios::pos_type ss = in.tellg();
  size_t fsize = (size_t)ss;
  int dim;
  in.seekg(0,std::ios::beg);
  in.read((char*)&dim, sizeof(int));
  size_t num = fsize / (dim+1) / 4;
  std::cout<<"load graph "<<num<<" "<<dim<< std::endl;
  in.seekg(0,std::ios::beg);
  tb.clear();
  for(size_t i = 0; i < num; i++){
    CandidateHeap heap;
    in.read((char*)&dim, sizeof(int));
    for(int j =0; j < dim; j++){
      int id;
      in.read((char*)&id, sizeof(int));
      Candidate<float> can(id, -1);
      heap.insert(can);
    }
    tb.push_back(heap);

  }
  in.close();
}

void saveResult(char* filename, vector<vector<int> > nn_results){
  std::ofstream out(filename,std::ios::binary);
  std::vector<std::vector<int> >::iterator i;
  //std::cout<<nn_results.size()<<std::endl;
  for(i = nn_results.begin(); i!= nn_results.end(); i++){
    std::vector<int>::iterator j;
    int dim = i->size();
    //std::cout<<dim<<std::endl;
    out.write((char*)&dim, sizeof(int));
    for(j = i->begin(); j != i->end(); j++){
      int id = *j;
      out.write((char*)&id, sizeof(int));
    }
  }
  out.close();

}


int main(int argc, char** argv){
  if(argc!=10){cout<<"arguments number error"<<endl;return -1;}
  Matrix func;
  Codes basecode;
  Codes querycode;
  Matrix train;
  Matrix test;
  cout<<"max support code length 32 and search hamming radius no greater than 1"<<endl;
  LoadHashFunc(argv[1],func);
  cout<<"load hash function complete"<<endl;
  LoadBaseCode(argv[2],basecode);
  cout<<"load base data code complete"<<endl;
  LoadData(argv[3],train);
  cout<<"load base data complete"<<endl;
  LoadData(argv[4],test);
  cout<<"load query data complete"<<endl;

  int UpperBits = 8;
  int LowerBits = 8; //change with code length:code length = up + low;
  HashTable tb;
  BuildHashTable(UpperBits,LowerBits,basecode,tb);
  cout<<"build hash table complete"<<endl;
  clock_t s,f;
  s = clock();

  QueryToCode(test, func, querycode);
  cout<<"convert query code complete"<<endl;
  vector<vector<int> > hashcands;
  HashTest(UpperBits, LowerBits, querycode, tb, hashcands);
  cout<<"hash candidates ready"<<endl;

  f = clock();
  cout<<"initial time : "<<(f-s)*1.0/CLOCKS_PER_SEC<<" seconds"<<endl;


  vector<CandidateHeap > knntable;
  L2Distance<float> distance_;
  LoadKnnTable(argv[5],knntable);
  cout<<"load knn graph complete"<<endl;
  //GNNS

  s = clock();
  vector<CandidateHeap > res;
  for(size_t i = 0; i < hashcands.size(); i++){
    CandidateHeap cands;
    for(size_t j = 0; j < hashcands[i].size(); j++){
      int neighbor = hashcands[i][j];
      Candidate<float> c(neighbor, distance_.compare(
          &test[i][0], &train[neighbor][0], test[i].size())
        );
      cands.insert(c);
      if(cands.size() > POOL_SIZE)cands.erase(cands.begin());
    }
    res.push_back(cands);
  }
  //iteration
  int expand = atoi(argv[6]);
  int iterlimit = atoi(argv[7]);
  for(size_t i = 0; i < res.size(); i++){
    int niter = 0;
    while(niter++ < iterlimit){
      CandidateHeap::reverse_iterator it = res[i].rbegin();
      vector<int> ids;
      for(int j = 0; it != res[i].rend() && j < expand; it++, j++){
        int neighbor = it->row_id;
        CandidateHeap::reverse_iterator nnit = knntable[neighbor].rbegin();
        for(int k = 0; nnit != knntable[neighbor].rend() && k < expand; nnit++, k++){
          int nn = nnit->row_id;
          ids.push_back(nn);
        }
      }
      for(size_t j = 0; j < ids.size(); j++){
        Candidate<float> c(ids[j], distance_.compare(
            &test[i][0], &train[ids[j]][0], test[i].size())
          );
        res[i].insert(c);
        if(res[i].size() > POOL_SIZE)res[i].erase(res[i].begin());
      }
    }//cout<<i<<endl;
  }
  cout<<"GNNS complete "<<endl;
  f = clock();
  cout<<"GNNS time : "<<(f-s)*1.0/CLOCKS_PER_SEC<<" seconds"<<endl;

  std::vector<std::vector<int> > resid;

  int knn_k = atoi(argv[8]);
  for(size_t i = 0; i < res.size(); i++){
    CandidateHeap::reverse_iterator it = res[i].rbegin();
    std::vector<int> vtmp;
    for(int j = 0; it != res[i].rend() && j < knn_k; it++, j++){
      vtmp.push_back(it->row_id);
    }
    resid.push_back(vtmp);
  }
  saveResult(argv[9], resid);

  return 0;
}
