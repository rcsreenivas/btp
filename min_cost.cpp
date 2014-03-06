#include<stdio.h>
#include<iostream>
#include<vector>
#include<set>
#include<algorithm>
#include<map>
#define MAXEVENTS 500

using namespace std;


//Structure to store subscription at some rsu
typedef struct {
	int start_time,end_time,rsu,id;
}subscriptions;


//Structure to store rsu info
typedef struct {
	int capacity,cost,id;
}rsu_struct;


//Struture to store chunk info
typedef struct {
	int rsu,coc,event,start_time,end_time,n,valid;
	double acoc,cbye;
	vector<int> f;
}chunk_struct;


vector<rsu_struct> rsu; //rsu[u] Stores the info about the rsu u
vector<vector<subscriptions > > sub; // sub[id] stores the vector having all of its rsu and the expected time internals of subscription id.
set<int> event[MAXEVENTS]; //event[e] stores all the subscribers that are subscribed to event e.
vector<vector<int> > cap_left;//cap_left[u][t] stores the capacity left at rsu u during the time slot t.
vector<vector<set<int > > > event_overlap; // event_overlap[u][t] stores all the events that are overlapping at rsu u during time slot t.
vector<int> reject; 
map<int ,int > rsu_map;
map<int ,int > sub_map;
map<int ,int > e_map;
vector<int> sub_event;//sub_event[i] stores its correspoing event;
vector<int> weight_chunks;//weight_chunks[u] stores the weight of the maximum wwighted chunk at rsu u.
vector<chunk_struct> max_chunks;//max_chunks[u] stores the maximum chunk at rsu u.
vector<vector<chunk_struct > > chunks; // chunks[e][u] stores all chunks of event e at rsu u.
vector<vector<set<int > > > e_rsu;//e_rsu[e][u] stores all the subscribers of event e at rsu u.
vector<map<int, subscriptions > >  s_rsu;//S_rsu[s] stores all tuples of the form <u, subscriptions> where u is the rsu present in the path of the subscriber s
int rsu_max,sub_no,rsu_no,time_slots,totalcost,sum=0,w1,w2,nume=-1,ev_no;



//Read all the rsu's
void read_rsus(char *filename){
	FILE* fp = fopen(filename,"r");
	fscanf(fp,"%d",&rsu_max);
	rsu.resize(rsu_max);
	rsu_struct temp_rsu;
	
	for(int i=0,temp,cap,cost,id; i < rsu_max; i++){
		fscanf(fp,"%d %d %d %d",&id,&temp,&cap,&cost);
		rsu[i].capacity = cap;
		rsu[i].cost = cost;
		rsu[i].id = i;
		rsu_map.insert(pair<int,int>(id,i));
	}
	cap_left.resize(rsu_max+1);
	event_overlap.resize(rsu_max+1);
	final.resize((rsu_max+1)*(sizeof(rsu_struct)));
	fclose(fp);
}


//Read all the subscriptions
void read_subscriptions(char *filename){
	
	int temp,s,e,u,st,et,max = -1;
	subscriptions temp_sub;
	FILE* fp = fopen(filename,"r");
	fscanf(fp,"%d %d %d %d %d",&temp,&temp,&ev_no,&sub_no,&rsu_no);
	sub.resize(sub_no);
	sub_event.resize(sub_no);
	s_rsu.resize(sub_no+1);
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
			sub_event[id] = j;
			e_map.insert(pair<int,int>(e,j));
			j++;
		}else {
			event[(*it).second].insert(id);	
			sub_event[id]=(*it).second;
		}
		s_rsu[id].insert(pair<int, subscriptions>(temp_sub.rsu, temp_sub));
		max = (max>et)?max:et;
	}
	time_slots = max;
	fclose(fp);
}

void init(){
	int i,j;
	totalcost = 0;
	
	for(i=0;i<rsu_max;i++) {
		cap_left[i].resize(time_slots+1);
		event_overlap[i].resize(time_slots+1);
	}
	for(i=0;i<rsu_max;i++)
		for(j=0;j<time_slots;j++)
			cap_left[i][j] = rsu[i].capacity;

	for(i=0;i<sub_no;i++){
		for(j=0;j<sub[i].size();k++)
			for(int e_id=sub[i][j].start_time;e_id<sub[i][j].end_time;e_id++) event_overlap[sub[i][j].rsu][e_id].insert(sub_event[sub[i][j].id]);
	}
}


bool start_time_sort(const subscriptions & lhs,const subscriptions & rhs ){
        return lhs.start_time < rhs.start_time;
}


double weight(chunk_struct ch){
	return w1*(ch.acoc);
}

void event_rsu_sub(){
	set<int>::iterator it;
	int sub_id;
	for(int i=0;i<ev_no;i++){
		for(it = event[i].begin();it!=event[i].end();it++){
			sub_id = *it;
			for(int k=0;k<sub[sub_id].size();k++){
				e_rsu[i][sub[sub_id][k].rsu].insert(sub_id);
			}	
		}
	}

}

void form_chunks(int e,int u){
	vector<subscriptions > temp;
	int sub_id,tf,df;
	double cbye;	
	chunk_struct chunk,max_chunk;	
	chunk.f.clear();
	max_chunk.f.clear();
	int min_tf,max_df,max,min;
	max_chunk.valid = 0;
	for(set<int>::iterator it=e_rsu[e][u].begin();it!=e_rsu[e][u].end();it++){
		sub_id = *it;
		temp.push_back(s_rsu[sub_id][u]);	
	}
	//Initialize max chunk and also take care if chunks are  empty
	if(!temp.empty())sort(temp.begin(),temp.end(),start_time_sort);
	else return;
	min_tf = temp[0].start_time;
	max_df = temp[0].end_time;	
	for(int i=0;i<temp.size();i++) {
		tf = temp[i].start_time;
		df = temp[i].end_time;
		if(tf>max_df){
			chunk.rsu = u;
			chunk.event = e;
			chunk.start_time = min_tf;
			chunk.end_time = max_df;
			chunk.coc = (rsu[u].cost)*(max_df-min_tf);
			chunk.acoc = chunk.coc/(float)chunk.f.size();
			chunk.n = chunk.f.size();
			max = -1;
			min = 100000;
			for(int k=min_tf;k<max_df;k++){
				min = (min<cap_left[u][k])?min:cap_left[u][k];
				if(min<=0) break;
				cbye = cap_left[u][k]/(float)(event_overlap[u][k].size());
				if(cbye < 1.0) max = (max>cbye)?max:cbye;
			}
			
			if(min>0){
				chunk.valid = 1;
				chunk.cbye = max;
				if(weight(max_chunk)<weight(chunk)) max_chunk = chunk; 
			}
			min_tf = tf;
			max_df = df;
			chunk.f.clear();
			chunk.f.push_back(temp[i].id);
		}else{
			chunk.f.push_back(temp[i].id);	
			max_df = (max_df > df)?max_df:df;
		}	
	}
	chunks[e][u] = max_chunk;
}

void min_cost(){
	chunks.resize(ev_no+1);
	max_chunks.resize(rsu_max+1);
	weight_chunks.resize(rsu_max+1);
	for(int i=0;i<ev_no;i++) chunks[i].resize(rsu_max+1);
	for(int i=0;i<ev_no;i++)
		for(int j=0;j<rsu_max;j++)	
			form_chunks(i,j);
	int j;
	double maxweight;
	int maxrsu,i;
	while(1){
		for(i=0;i<rsu_max;i++){
  	              for(j=0;j<ev_no;j++){
        	                if(chunks[i][j].valid == 1){
                	                max_chunks[i] = chunks[i][0];
                        	        weight_chunks[i] = weight(max_chunks[i]);
                               		break;
                        	}
                	}
                	if(j == ev_no) {
                        	max_chunks[i].valid = 0;
                        	continue;
                	}
                	for(;j<ev_no;j++){
                        	if(chunks[i][j].valid == 0) continue;
                        	if(weight(chunks[i][j])>weight_chunks[i]){
                                	weight_chunks[i] = weight(chunks[i][j]);
                                	max_chunks[i] = chunks[i][j];
                        	}
                	}
        	}

		maxrsu = -1;
		for(i=0;i<rsu_max;i++) {
			if(max_chunks[i].valid == 1){
				maxweight = weight_chunks[i];
				maxrsu = i;	
				break;
			}
		}
		

		for(;i<rsu_max;i++){
			if(max_chunks[i].valid == 1){
				if(maxweight > weight_chunks[i]){
					maxweight = weight_chunks[i];
					maxrsu = i;
				}
			}
		}

		if(maxrsu == -1) break;
		totalcost += max_chunks[maxrsu].coc;
		for(i=max_chunks[maxrsu].start_time;i<max_chunks[maxrsu].end_time;i++){
			event_overlap[maxrsu][i].erase(max_chunks[maxrsu].event);
			cap_left[maxrsu][j]-=1;
		}	
		
		for(i=0;i<max_chunks[maxrsu].f.size();i++){
			e_rsu[max_chunks[maxrsu].event][maxrsu].erase(max_chunks[maxrsu].f[i]);
		}

		int ev = max_chunks[maxrsu].event;
		
		for(i=0;i<ev_no;i++) form_chunks(i,maxrsu);
		for(i=0;i<rsu_max;i++) form_chunks(ev,i);
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
