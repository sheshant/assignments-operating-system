#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <algorithm>
#include <string>
#include <fstream>

using namespace std;

typedef struct bit
{
	int bit_data;
	int ref_str;
}bit;

void display(deque<bit>& d)
{
	int size = d.size();
	for (int i = 0; i < d.size(); ++i)
	{
		bit display_bit = d[i];
		cout<<display_bit.ref_str<<" "<<display_bit.bit_data<<endl;
	}
	cout<<endl;
}
vector<int> reference_string(int n,int d)
{
	vector<int> ref_str = vector<int>(0);
	int first_matrix[n][n];
	int second_matrix[n][n];
	int output_matrix[n][n];
	int fm = 100,sm = 500,om = 1000;
	int count = 0,add = 1;
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			first_matrix[i][j] = fm+add;
			second_matrix[i][j] = sm+add;
			output_matrix[i][j] = om+add;
			count++;
			if(count % d == 0)
			{
				add ++;
			}
		}
	}
	for (int i = 0; i < n; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			for (int k = 0; k < n; ++k)
			{
				ref_str.push_back(first_matrix[i][k]);
				ref_str.push_back(second_matrix[k][j]);
			}
			ref_str.push_back(output_matrix[i][j]);
		}
	}
	return ref_str;
}
int fcfs(int d,const vector<int>& vec)
{
	deque<int> fcfs_deque;
	int size = vec.size(),new_page= 0,page_fault = 0,deque_size = fcfs_deque.size();
	for (int i = 0; i < size; ++i)
	{
		new_page = vec.at(i);
		if(find(fcfs_deque.begin(), fcfs_deque.end(), new_page) == fcfs_deque.end())
		{
			page_fault ++;
			fcfs_deque.push_back(new_page);
			deque_size ++;
			if(deque_size > d)
			{
				fcfs_deque.pop_front();
				deque_size--;
			}
		}
	}
	return page_fault;
}
int lru(int d,const vector<int>& vec)
{
	deque<int> lru_deque;
	deque<int>::iterator it;
	int size = vec.size(),new_page= 0,page_fault = 0,deque_size = lru_deque.size();
	for (int i = 0; i < size; ++i)
	{
		new_page = vec.at(i);
		if((it = find(lru_deque.begin(), lru_deque.end(), new_page)) == lru_deque.end())
		{
			page_fault ++;
			lru_deque.push_front(new_page);
			deque_size ++;
			if(deque_size > d)
			{
				lru_deque.pop_back();
				deque_size--;
			}
		}
		else
		{
			lru_deque.erase(it);
			lru_deque.push_front(new_page);
		}
	}
	return page_fault;
}
int second_chance(int d,const vector<int>& vec)
{
	deque<bit> sc_deque;
	deque<bit>::iterator it;
	int size = vec.size(),new_page= 0,page_fault = 0,deque_size = sc_deque.size();
	// iterate a loop over all elements of the reference string
	for (int i = 0; i < size; ++i)
	{
		new_page = vec.at(i);
		// cout<<"new page = "<<new_page<<endl;
		// here is the new page
		// we need to check whether it is there in the deque or not 
		bool found = false;
		for(deque<bit>::iterator itr = sc_deque.begin();itr != sc_deque.end(); ++itr)
		{
			if((*itr).ref_str == new_page)
			{
				found = true;
				// we found it here so the page is referenced so we need to set the bit data to 1
				// cout<< "no page fault" <<endl;
				(*itr).bit_data = 1;
				break;
			}
		}
		// in case if we didn't find
		if(found == false)
		{
			// then we need to insert the new bit as there is a page fault
			page_fault++;
			// cout<< "page fault" <<endl;
			bit new_bit;
			new_bit.ref_str = new_page;
			new_bit.bit_data = 0;
			// case 1 in case if the size of deque is less than the d
			if(deque_size < d)
			{
				// cout<<"before"<<endl;
				// display(sc_deque);
				// then we need to simply insert it 
				deque_size++;
				sc_deque.push_front(new_bit);
				// cout<<"after"<<endl;
				// display(sc_deque);
			}
			// case 2 then we need to find the victim page 
			else
			{
				// cout<<"before"<<endl;
				// display(sc_deque);
				// in case if the last bit is set 1
				bit last_bit = sc_deque.back();
				sc_deque.pop_back();
				while(last_bit.bit_data == 1)
				{
					// then we need to set it 0 and insert it
					last_bit.bit_data = 0;
					sc_deque.push_front(last_bit);
					last_bit = sc_deque.back();
					sc_deque.pop_back();
				}
				// we keep removing last bit until we found ref bit zero which we popped 
				// now just we need to store newbit
				sc_deque.push_front(new_bit);
				// cout<<"after"<<endl;
				// display(sc_deque);
			}
		}
	}
	return page_fault;
}
int lfu(int d,const vector<int>& vec) 
{
	deque<bit> lfu_deque;
	deque<bit>::iterator it;
	int size = vec.size(),new_page= 0,page_fault = 0,deque_size = lfu_deque.size();
	// iterate a loop over all elements of the reference string
	for (int i = 0; i < size; ++i)
	{
		new_page = vec.at(i);
		// cout<<"new page = "<<new_page<<endl;
		// here is the new page
		// we need to check whether it is there in the deque or not 
		bool found = false;
		for(deque<bit>::iterator itr = lfu_deque.begin();itr != lfu_deque.end(); ++itr)
		{
			if((*itr).ref_str == new_page)
			{
				found = true;
				// we found it here so the page is referenced so we need to set the bit data to 1
				// cout<< "no page fault" <<endl;
				(*itr).bit_data ++;
				break;
			}
		}
		// in case if we didn't find
		if(found == false)
		{
			// then we need to insert the new bit as there is a page fault
			page_fault++;
			// cout<< "page fault" <<endl;
			bit new_bit;
			new_bit.ref_str = new_page;
			new_bit.bit_data = 0;
			// case 1 in case if the size of deque is less than the d
			if(deque_size < d)
			{
				// cout<<"before"<<endl;
				// display(lfu_deque);
				// then we need to simply insert it 
				deque_size++;
				lfu_deque.push_front(new_bit);
				// cout<<"after"<<endl;
				// display(lfu_deque);
			}
			else
			{
				// cout<<"before"<<endl;
				// display(lfu_deque);
				// now we need to iterate and find the bit with the least count
				int ref_count_lfu = 1000;
				for(deque<bit>::iterator itr = lfu_deque.begin();itr != lfu_deque.end(); ++itr)
				{
					if((*itr).bit_data <ref_count_lfu)
					{
						ref_count_lfu = (*itr).bit_data;
						it = itr;
					}
				}
				// we found the bit with the least number of references now delete it
				// cout<<"least bit = "<<(*it).ref_str<<endl;
				lfu_deque.erase(it);
				// now just we need to store newbit
				lfu_deque.push_front(new_bit);
				// cout<<"after"<<endl;
				// display(lfu_deque);
			}
		}
	}
	return page_fault;
}
void plot_graph(int a,int b,const vector<int>& vec,int choice)
{
	string file = "output"+to_string(choice)+".txt";
	ofstream out(file);
	vector<string> commands = vector<string>(0);
	string line = "";
	string filename = "";
	switch(choice)
	{
		case (1):
		{
			for (int i = a; i <= b; ++i)
			{
				int value = fcfs(i,vec);
				line = to_string(i) + "\t\t" + to_string(value) +"\n";
				out << line;
			}
			commands.push_back("set title 'FIFO implementation pagefault vs frame length'");
			filename = "FCFS.png";
			break;
		}
		case (2):
		{
			for (int i = a; i <= b; ++i)
			{
				int value = lfu(i,vec);
				line = to_string(i) + "\t\t" + to_string(value) +"\n";
				out << line;
			}
			commands.push_back("set title 'LFU implementation pagefault vs frame length'");
			filename = "LFU.png";
			break;
		}
		case (3):
		{
			for (int i = a; i <= b; ++i)
			{
				int value = lru(i,vec);
				line = to_string(i) + "\t\t" + to_string(value) +"\n";
				out << line;
			}
			commands.push_back("set title 'LRU implementation pagefault vs frame length'");
			filename = "LRU.png";
			break;
		}
		case (4):
		{
			for (int i = a; i <= b; ++i)
			{
				int value = second_chance(i,vec);
				line = to_string(i) + "\t\t" + to_string(value) +"\n";
				out << line;
			}
			commands.push_back("set title 'Second chance implementation pagefault vs frame length'");
			filename = "second_chance.png";
			break;
		}
		default :break;
	}
	out.close();
	// now we need to plot output.txt
	FILE * gnuplotPipe = popen ("gnuplot -persistent", "w");
	commands.push_back("set ylabel 'page faults';");
	commands.push_back("set xlabel 'frame number';");
	commands.push_back("set terminal png size 1200,700 enhanced font 'Helvetica,20'");
	commands.push_back("set output '"+filename+"';");
	commands.push_back("plot '"+file+"' using 1:2 with lines linetype 3 title 'output';");
	for (int i = 0; i < commands.size(); ++i)
	{
		fprintf(gnuplotPipe, "%s \n", commands.at(i).c_str());
	}
}
int main()
{
	int n,d,a,b,choice;

	cout<<"n = ";
	cin>>n;
	cout<<"d = ";
	cin>>d;
	
	cout<<endl;
	vector<int> r = reference_string(n,d);
	while(1)
	{
		cout<<"Enter your choice"<<endl;
		cout<<"1 : FIFO"<<endl;
		cout<<"2 : LFU"<<endl;
		cout<<"3 : LRU"<<endl;	
		cout<<"4 : Second chance algorithm"<<endl;
		cout<<"5 : EXIT"<<endl;
		cin>>choice;
		switch(choice)
		{
			case (1):
			{
				cout<<"Range of pages will be from a to b \nEnter both values"<<endl;
				cout<<"a = ";
				cin>>a;
				cout<<"b = ";
				cin>>b;
				plot_graph(a,b,r,1);break;
			}
			case (2):
			{
				cout<<"Range of pages will be from a to b \nEnter both values"<<endl;
				cout<<"a = ";
				cin>>a;
				cout<<"b = ";
				cin>>b;
				plot_graph(a,b,r,2);break;
			}
			case (3):
			{
				cout<<"Range of pages will be from a to b \nEnter both values"<<endl;
				cout<<"a = ";
				cin>>a;
				cout<<"b = ";
				cin>>b;
				plot_graph(a,b,r,3);break;
			}
			case (4):
			{
				cout<<"Range of pages will be from a to b \nEnter both values"<<endl;
				cout<<"a = ";
				cin>>a;
				cout<<"b = ";
				cin>>b;
				plot_graph(a,b,r,4);break;
			}
			case (5):exit(0);break;
			default : cout<<"Please enter again"<<endl;break;
		}
	}
	return 0;
}