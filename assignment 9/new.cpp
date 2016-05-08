#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <algorithm>
#include <curl/curl.h>
#include <htmlcxx/html/ParserDom.h>
#include <unistd.h>
using namespace std;
using namespace htmlcxx;

static size_t __writeCallBack(void *, size_t , size_t , void *);
vector<string> __getLinks(string);
bool __hasEnding (std::string const &, std::string const &);

typedef struct label
{
	int thread_id;
	int level_id;
}label;

// global variables
queue<string> to_do = queue<string>();
queue<string> to_do_next = queue<string>();
vector<string> done_list = vector<string>(0);
vector<string> crawl_list = vector<string>(0);
vector<label> label_list = vector<label>(0);
vector<int> thread_id = vector<int>(0);
int url_level = 0,i,no_of_times,nthreads = 0;
// global mutex
pthread_mutex_t donemutex;  /// we are using three variables because ????????    my choice
pthread_mutex_t to_do_mutex;
pthread_mutex_t donemutex2;
pthread_mutex_t queue_mutex;
// global condition
pthread_cond_t finish_cond;
pthread_attr_t attr;
char * u;

void * func (void * threadid)
{
	int *id_ptr, taskid;
	usleep(10);
	id_ptr = (int *) threadid;
	taskid = *id_ptr;
	// got the thread id now start the process
	string to_do_url = "null";
	pthread_mutex_lock(&donemutex2); // for while
	while(url_level <no_of_times)
	{
		pthread_mutex_unlock(&donemutex2);// for while
		pthread_mutex_lock(&queue_mutex);// for if() we need to check whether it is in done or not
		if(! to_do.empty())
		{
			// pthread_mutex_lock(&to_do_mutex);
			string to_do_url = to_do.front();
			to_do.pop();
			pthread_mutex_unlock(&queue_mutex);// for if() we need to check whether it is in done or not
			pthread_mutex_lock(&to_do_mutex);// for crawl lock
			// here we are going to check whether the url is new or not 
			if(find(done_list.begin(), done_list.end(), to_do_url) == done_list.end())
			{
				printf("got new url (%s) by thread id %d\n",to_do_url.c_str(),taskid);
				// new url we got it now we need to crawl
				done_list.push_back(to_do_url);
				vector<string> s = __getLinks(to_do_url);
				for(i = 0;i<s.size();i++)
				{
					string crawl_string = s.at(i);// the string url we got after crawling
					to_do_next.push(crawl_string);
					if(find(crawl_list.begin(), crawl_list.end(), crawl_string) == crawl_list.end())
					{
						// after crawling we are going to add it in the new crawl list
						crawl_list.push_back(crawl_string);
						// and also we are going to update the label list
						label l;
						l.thread_id = taskid;
						l.level_id = url_level+1;
						label_list.push_back(l);
						// updated done exit the list
					}
				}
			}
			else
			{

			}
			
			// usleep(10000);
			pthread_mutex_unlock(&to_do_mutex);// for crawl
		}
		else
		{
			pthread_mutex_unlock(&queue_mutex);// for if() we need to check whether it is in done or not
			// pthread_mutex_unlock(&to_do_mutex);
			printf("thread %d at mutex\n",taskid );
			pthread_mutex_lock(&donemutex);
			nthreads++;
			if(nthreads != 5)
			{
				printf("thread %d at mutex going to wait nthreads = %d\n",taskid,nthreads );
				pthread_cond_wait(&finish_cond, &donemutex);
				// need to unlock this mutex because after condition wait mutex is again locked
				pthread_mutex_unlock(&donemutex);
			}
			else
			{
				printf("thread %d at mutex going to broadcast\n",taskid );
				pthread_cond_broadcast(&finish_cond);
				// copy contents from one queue to another Copy to-do-next to to-do and increase level
				while(!(to_do_next.empty()))
				{
					to_do.push(to_do_next.front());
					to_do_next.pop();
				}
				url_level++;
				printf("url level increased to %d\n",url_level );
				nthreads = 0;
				pthread_mutex_unlock(&donemutex);
			}
		}
	}
	pthread_mutex_unlock(&donemutex2);// for while in case if remains here 

	printf("%d\n",taskid);
	pthread_exit(NULL);
}
int main()
{
	pthread_t threads[5];
	cout<<"Enter total depth of the bfs"<<endl;
	cin>>no_of_times;
	u = (char*)malloc(100*sizeof(char));

	/* Initialize mutex and condition variable objects */
	pthread_mutex_init(&donemutex, NULL);// we may require only one mutex
	pthread_mutex_init(&to_do_mutex, NULL);
	pthread_mutex_init(&donemutex2, NULL);
	pthread_mutex_init(&queue_mutex, NULL);
	pthread_cond_init (&finish_cond, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);// set attribute joinable
	to_do.push("http://cse.iitkgp.ac.in/");
	// pthread_t threads[5];
	int *taskids[5];
	int thread_id;
	
	for (i = 0; i < 5; ++i)
	{
		taskids[i] = (int *) malloc(sizeof(int));
  		*taskids[i] = i+1;
		thread_id = i+1;

		/* Create thread with number and name passed as parameters */
		printf("thread id = %d\n",thread_id );
		if(pthread_create(&threads[i],&attr, func, (void *)(taskids[i]))) 
		{
			fprintf(stderr, "Master thread: Unable to create thread\n");
			pthread_attr_destroy(&attr);
			exit(1);
		}
	}

	for (i = 0; i < 5; ++i)
	{
		pthread_join(threads[i],NULL);
	}
	int length = crawl_list.size();
	cout<<"<level> \t" <<"<thread> \t" << "url"<<endl;
	for (i = 0; i < length; ++i)
	{
		cout<< label_list.at(i).level_id<<"\t"<<label_list.at(i).thread_id<<"\t" <<crawl_list.at(i) <<endl;
	}

	/* Clean up and exit */
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&donemutex);
	pthread_mutex_destroy(&to_do_mutex);
	pthread_mutex_destroy(&donemutex2);
	pthread_mutex_destroy(&queue_mutex);
	pthread_cond_destroy(&finish_cond);
	pthread_exit (NULL);
	return 0;
}
static size_t __writeCallBack(void *contents, size_t size, size_t nmemb, void *userp){
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
vector<string> __getLinks(string ip){

	char url[2048];
	bzero(url,2048);
	strcpy(url,ip.c_str());

	string url_s(url);
	string prefix;
	
	if(__hasEnding(url_s,".html")){
		int pos1  = url_s.find("//");
		int pos2  = url_s.find("/",pos1+2);
		prefix = url_s.substr(0,pos2+1);
		goto code;
	}

	if(url[strlen(url)-1]!='/'){
		strcat(url,"/");
		string check(url);
		int pos1  = check.find("//");
		int pos2  = check.find("/",pos1+2);
		prefix = check.substr(0,pos2+1);
		goto code;
	}
	else{
		string check(url);
		int pos1  = check.find("//");
		int pos2  = check.find("/",pos1+2);
		prefix = check.substr(0,pos2+1);
		goto code;
	}

	code:
	CURL *curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __writeCallBack);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    HTML::ParserDom parser;
 	tree<HTML::Node> dom = parser.parseTree(readBuffer);

  	//Dump all links in the tree
  	tree<HTML::Node>::iterator it = dom.begin();
  	tree<HTML::Node>::iterator end = dom.end();
  	vector<string> op;
  	
  	for (; it != end; ++it){

  		if (it->tagName() == "a"){

        	it->parseAttributes();
        	string link = it->attribute("href").second;

        	if( link !="" && link !="#" && link.substr(0,6)!="mailto"){

        		// process on links
        		if(link.substr(0,4)=="http"){
        			op.push_back(link);
        		}
        		else{
        			string psh = prefix+link;
        			op.push_back(psh);
        		}
        		/*
        		else if(link.substr(0,10)=="index.php?"){
        			string prefix(url);
        			if(prefix.find("index.php?")!=string::npos && link.find("index.php?")!=string::npos){
        				int lastpos = prefix.find("index.php?");
        				link = prefix.substr(0,lastpos) + link;
        			}
        			else{
        				link = prefix+link;
        			}
        			op.push_back(link);
        		}
        		else if(__hasEnding(link,".html")){
        			link = url+link;
        			op.push_back(link);
        		}
        		
        		else {
        			cout << "can not decode : " << link << endl;
        			exit(0);
        		}
        		*/
        		
        	}
    	}
  	}

  	// cout << "----------- Printing links -------------------" << endl;
  	// for (int i=0;i<op.size();i++){
  	// 	cout << "(" << i+1  << ")\t" << op.at(i) << endl;
  	// }
  	return op;

}
bool __hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}