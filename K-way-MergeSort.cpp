#include <bits/stdc++.h>
using namespace std;
const int MAX_MEMORY = 900*1024*1024; // 512 MB

void create_initial_runs(const char* input);
int intermediate_merge(int stage , int total_runs , int k , const char *output , int &ans 
    , const int &num_merges);

int external_merge_sort_withstop(const char *input , const char *output,
	const long key_count, const int k = 2 , const int num_merges = 0) ;

/*returns total initial runs */

int create_initial_runs(const char* input, const long key_count){
	ifstream input_file(input);
	string s;
	int run_num = 1;
    long line = 0 , sz = MAX_MEMORY/1024;
    vector<string> buffer(sz); /*this will store the ith run */
	while(input_file && line<key_count){
		
		int i = 0;
		/*run size should be less than the available memory */
        string s;
		while(line<key_count && i<sz && getline(input_file,s)){
            buffer[i++] = s;
            line++;
		}
        sort(buffer.begin(),buffer.begin() + i);
		/*write the buffer to the file*/
		string run_filename = "temp." + to_string(0) + "." + to_string(run_num)
							+ ".txt";
        ofstream run_file(run_filename);
        for (int j = 0 ; j<i;j++) {
            run_file << buffer[j] << '\n';
        }
        run_file.close();
        run_num++;
	}
    input_file.close();
    return run_num -1;
}


/* It returns the numbers of runs for the next stage */
int intermediate_merge(int stage , int total_runs , int k , const char *output , int &ans , 
    const int &num_merges){

    /*allocation of M/(k + 1) memory for each run including the buffer for output*/
    int buffmem = MAX_MEMORY/(min(k + 1,total_runs + 1));

    /*check if buffmem is atleast equal to size of a longest key */
    while(buffmem < 2050){
        k = k/2;
        buffmem = MAX_MEMORY/(min(k + 1,total_runs + 1));
    }

    if(k <= 0 || (k<=1 and total_runs > 1)){
        return -1;
    }

    /*calculates total runs for next stage */
    int total_runs_new = 0 ;
    /*loop through each k runs*/
    for(int i = 1; i<=total_runs;i+=k){
        ans++;
        string output_file;
        if(total_runs > k){
            output_file = "temp." + to_string(stage) + "." + to_string((i + k-1)/k) + ".txt";
        }
        else{
            output_file = output;
        }
        ofstream out(output_file);

        total_runs_new++;

        auto cmp = [](const pair<string, int>& p1, const pair<string, int>& p2) {
                return p1.first > p2.first;
        };

        priority_queue<pair<string, int>, vector<pair<string, int>>, decltype(cmp)> pq(cmp);
        vector<ifstream> input_files;

        vector<int> run_sizes(k); /*stores the current size of each run */

        int buff_size = buffmem/1024;
        vector<string> buffer(buff_size);
        int sz = -1;

        /*store the pointers to input files*/
        for(int j = i;j<min(i + k,total_runs + 1) ; j++){
            string filename = "temp." + to_string(stage -1) + "." + to_string(j) + ".txt";
            input_files.emplace_back(ifstream(filename));
        }

        /*Read the initial entries of each input files*/

        for(int j = i;j<min(i + k,total_runs + 1) ; j++){
            int run_size = 0;
            string s;
            while((run_size + 1025)<buffmem && getline(input_files[(j-1) % k] , s)){
                pq.push({s,(j-1)%k});
                run_size += (s.size() + sizeof(int));
            }
            run_sizes[(j - 1)%k] = run_size;
        }


        while(!pq.empty()){
            /* Get the smallest element in the priority queue */
            pair<string, int> p = pq.top();
            pq.pop();

            run_sizes[p.second] -= (p.first.size() + sizeof(int));


            /* Write the smallest element to the buffer */
            sz += 1;

            /*write the buffer to output if it size exceeds the limit */
            if(sz >= buff_size){
                for(auto s:buffer){
                    out << s << '\n';
                }
                sz = 0;
            }
            buffer[sz] = p.first;


            /* Get the next elements if the memory for the run 
            which had current smallest is totally 
            free*/
            if(run_sizes[p.second] == 0){
                int buff_size = 0;
                string s;
                while((buff_size + 1025)<buffmem && getline(input_files[p.second],s)){
                    pq.push({s,p.second});
                    buff_size += (s.size() + sizeof(int));
                }
                run_sizes[p.second] = buff_size;
            }
        }
        /*write all the remaining entries to output */
        for(int j = 0 ; j<=sz;j++){
            out << buffer[j] << '\n';
        }
        buffer.clear();
        for(ifstream& x:input_files){
            x.close();
        }
        out.close();

        if(ans == num_merges){
            return total_runs_new;
        }
    }

    return total_runs_new;

}





/*returns the number of merge-steps completed*/
int external_merge_sort_withstop( const char *input , const char *output ,
	const long key_count, const int k, const int num_merges){
    int total_runs = create_initial_runs(input , key_count);
    int ans = 0;

    // cout<<"Total runs for the stage "<<0<< " are " <<total_runs<<"\n";

    /* Do intermediate merges until total_runs is greater than k */
    int stage = 1;
    if(total_runs == 1){
        intermediate_merge(stage , total_runs , k ,output, ans , num_merges);
        return 1;
    }
    while(total_runs > 1){
        total_runs = intermediate_merge(stage , total_runs , k ,output , ans , num_merges);
        // cout<<"Total runs for the stage "<<stage<< " are " <<total_runs<<"\n";
        if(total_runs == -1){
            return -1;
        }
        if(num_merges != 0 && ans >= num_merges){
            break;
        }
        stage++;
    }
    return ans;
}


// int main(){
//     const char* input = "random.txt";
//     const char* output = "sorted_output1.txt";
//     const long key_count = 10000000000000;
//     const int k = 2;
//     const int num_merges = 10;

//     int total_merge_step = external_merge_sort_withstop(input,output,key_count,k,num_merges);
//     cout<<"Number of Merge-Steps took: "<<total_merge_step<<"\n";
// }