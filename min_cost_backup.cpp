#include<stdio.h>
#include<iostream>
#include<vector>
#include<set>
#include<algorithm>
#include<map>
#define MAXEVENTS 500

using namespace std;

typedef struct {
	int start_time,end_time,rsu,id;
}subscriptions;

typedef struct {
	int capacity,cost,id;
}rsu_struct;

typedef struct {
	int rsu,coc,start_time,end_time,rc;
	double acoc;
	vector<int> f;
}chunk_struct;

vector<rsu_struct> rsu;
vector<vector<subscriptions > > sub;
set<int> event[MAXEVENTS];
vector<vector<int> > cap_left;
vector<int> reject;
vector<chunk_struct > final;
vector<chunk_struct> chunks;
map<int ,int > rsu_map;
map<int ,int > sub_map;
map<int ,int > e_map;
int rsu_max,sub_no,rsu_no,time_slots,totalcost,sum=0,w1,w2,nume=-1,ev_no;
void read_rsus(char *filename){
	FILE* fp = fopen(filename,"r");
	fscanf(fp,"%d",&rsu_max);
	rsu.resize(rsu_max);
	rsu_struct temp_rsu;
	//printf("%d\n",rsu.size());
	for(int i=0,temp,cap,cost,id; i < rsu_max; i++){
		fscanf(fp,"%d %d %d %d",&id,&temp,&cap,&cost);
		//printf("%d %d\n",cap,i);
		rsu[i].capacity = cap;
		rsu[i].cost = cost;
		rsu[i].id = i;
		rsu_map.insert(pair<int,int>(id,i));
	}
	cap_left.resize(rsu_max+1);
	final.resize((rsu_max+1)*(sizeof(rsu_struct)));
	fclose(fp);
}

void read_subscriptions(char *filename){
	int temp,s,e,u,st,et,max = -1;
	subscriptions temp_sub;
	FILE* fp = fopen(filename,"r");
	fscanf(fp,"%d %d %d %d %d",&temp,&temp,&ev_no,&sub_no,&rsu_no);
	sub.resize(sub_no);
	int i = 0,j = 0,id;
	map<int,int>::iterator it;
	while(fscanf(fp,"%d %d %d %d %d",&s,&e,&u,&st,&et) != -1){
		nume = (nume>e)?nume:e;
		temp_sub.start_time = st;
		temp_sub.end_time = et;
		temp_sub.rsu = rsu_map[u];
		sum+=rsu[temp_sub.rsu].cost;
		it = sub_map.find(s);
		if(it == sub_map.end()){
			id = i;
			temp_sub.id = id;
			sub[i].push_back(temp_sub);
			sub_map.insert(pair<int,int>(s,i));
			i++;
		}else {
			temp_sub.id = (*it).second;
			sub[(*it).second].push_back(temp_sub);
			id = (*it).second;
		}
		it = e_map.find(e);
		if(it == e_map.end()){
			event[j].insert(id);
			e_map.insert(pair<int,int>(e,j));
			j++;
		}else event[(*it).second].insert(id);	
		max = (max>et)?max:et;	
	}
	time_slots = max;
	fclose(fp);
}

void init(){
	int i,j;
	totalcost = 0;
	
	for(i=0;i<rsu_max;i++) cap_left[i].resize(time_slots+1);
	for(i=0;i<rsu_max;i++)
		for(j=0;j<time_slots;j++)
			cap_left[i][j] = rsu[i].capacity;
}

bool start_time_sort(const subscriptions & lhs,const subscriptions & rhs ){
	return lhs.start_time < rhs.start_time;
}

bool weight(const chunk_struct & lhs,const chunk_struct & rhs){
	return (w1*lhs.acoc - w2*lhs.rc) < (w1*rhs.acoc - w2*rhs.rc);
}
void min_cost(){
	vector<vector<subscriptions> > temp;
	int sub_id,max_df,tf,df,min_tf;
	int min;
	set<int>::iterator it;
	chunk_struct chunk;
	vector<chunk_struct> chunks;
	temp.resize(rsu_max);
	for(int i=0;i<ev_no;i++){
		while(!event[i].empty()){
			for(int j=0;j<rsu_max;j++) temp[j].clear();
			for(it = event[i].begin();it!=event[i].end();it++){
				sub_id = *it;		
				for(int k=0;k<sub[sub_id].size();k++)
					temp[sub[sub_id][k].rsu].push_back(sub[sub_id][k]);
			}
			chunks.clear();
			for(int j = 0; j<rsu_max;j++) if(!temp[j].empty())sort(temp[j].begin(),temp[j].end(),start_time_sort);
		
		//	for(int j=0;j<rsu_max;j++) for(int k=0;k<temp[j].size();k++) printf("%d %d %d %d \n",j,temp[j][k].id,temp[j][k].start_time,temp[j][k].end_time);
		//	break;			
			for(int j=0;j<rsu_max;j++) {
				if(temp[j].empty()) continue;
				min_tf = temp[j][0].start_time;
				max_df = temp[j][0].end_time;			
				for(int k=0;k<temp[j].size();k++){
					tf = temp[j][k].start_time;
					df = temp[j][k].end_time;
					if(tf>max_df){
						chunk.rsu = j;
						chunk.start_time = min_tf;
						chunk.end_time = max_df;
						chunk.coc = (rsu[j].cost)*(max_df-min_tf);
						chunk.acoc = chunk.coc/(float)chunk.f.size();
						min = 10000;
						for(int m=min_tf;m<max_df;m++) min = (min<cap_left[j][m])?min:cap_left[j][m];
						chunk.rc = min;
						if(min > 0)chunks.push_back(chunk);
						min_tf = tf;
						max_df = df;
						chunk.f.clear();
						//printf("%d rcs\n",temp[j][k].id);
						chunk.f.push_back(temp[j][k].id);												
					}else{
						chunk.f.push_back(temp[j][k].id);
						max_df = (max_df > df)?max_df:df;
					}			
				
				}
				chunk.rsu = j;
                                chunk.start_time = min_tf;
                                chunk.end_time = max_df;
                                chunk.coc = (rsu[j].cost)*(max_df-min_tf);
                                chunk.acoc = chunk.coc/(float)chunk.f.size();
                               	min = 10000;
                              	for(int m=min_tf;m<max_df;m++) min = (min<cap_left[j][m])?min:cap_left[j][m];
                                if(min > 0) {
					chunk.rc = min;
                                	chunks.push_back(chunk);
				}
				chunk.f.clear();

			}
			if(chunks.empty()) {
				for(it = event[i].begin();it!=event[i].end();it++) reject.push_back(*it);
				break;
			}
			sort(chunks.begin(),chunks.end(),weight);
			chunk = chunks[0];
			totalcost+=chunk.coc;
			for(int j=chunk.start_time;j<chunk.end_time;j++) cap_left[chunk.rsu][j]-=1;
			//for(int j=0;j<chunk.f.size();j++) printf("%d %d %d %d\n",chunk.rsu,chunk.f[j],chunk.start_time,chunk.end_time);
			final.push_back(chunks[0]);
			//for(int j=0;j<chunk.f.size();j++) printf("%d -erase\n",event[i].erase(chunk.f[j]));	
			for(int j=0;j<chunk.f.size();j++) event[i].erase(chunk.f[j]);
			chunk.f.clear();
		}
	}
}


int main(int argc, char *argv[]){
	if(argc == 5){
		w1 = atoi(argv[3]);
		w2 = atoi(argv[4]);
		read_rsus(argv[1]);
		read_subscriptions(argv[2]);
		init();
		//for(int i=0;i<rsu_max;i++) printf("%d %d %d\n",rsu[i].id,rsu[i].capacity,rsu[i].cost);
		//for(int i=1;i<=sub_no;i++) for(int j=0;j<sub[i].size();j++)printf("%d %d %d %d\n",sub[i][j].id,sub[i][j].rsu,sub[i][j].start_time,sub[i][j].end_time);
		//for(int i=0;i<rsu_max;i++) {for(int j=0;j<time_slots;j++) printf("%d ",cap_left[i][j]);printf("\n");}
		min_cost();
		int sat = sub_no - reject.size();
		printf("%d %d %d %f %f \n",nume,rsu_no,sub_no,totalcost/(float)sat,sat*100/(float)sub_no);
		//for(int i=0;i<rsu_max;i++) for(int j=0;j<time_slots;j++) if(cap_left[i][j] <=0) printf("%d ",cap_left[i][j]);
		//for(int i=0;i<final.size();i++) for(int j=0;j<final[i].f.size();j++) printf("%d %d %d %d\n",final[i].rsu,final[i].f[j],final[i].start_time,final[i].end_time);
	}

	
	return 0;
}
