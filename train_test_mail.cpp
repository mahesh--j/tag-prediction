#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include<cstdlib>
#include<cctype>
#include<algorithm>
#include<vector>

#define NUMBER 500
#define PREDICT 100

#define MAX_TAGS 10 //Maximum tags per question

using namespace std;

struct learnings{
	int tag_in_title[MAX_TAGS];
	int body_count[MAX_TAGS];
};
typedef struct learnings learnt;

class question{
	long id;
	string title;
	string body;
	string tag_list;
	learnt l;
	
	int tag_number;
	
	public:
		void learn();
		void processTitle(string current_tag);
		void processBody(string current_tag);
		void analyzeCode();
				
		void setId(long l){
			id= l;
		}
		
		void setTitle(string s){
			title= s;
		}

		void setBody(string s){
			body= s;
		}

		void setTagList(string s){
			tag_list= s;			
		}

		string getTitle(){
			return title;
		}

		long getId(){
			return id;
		}

		string getBody(){
			return body;
		}

		string getTagList(){
			return tag_list;
		}

		learnt getLearnings(){
			return l;
		}

};

void question::learn(){
	istringstream iss(tag_list);
	string token;

	tag_number=0;
	while( getline(iss,token,' ') ){
		processTitle(token);
		processBody(token);
		tag_number++;
	}
}

//counts number of occurences of current_tag in question body
void question::processBody(string current_tag){
	int pos;
	int count=0;
	istringstream iss(current_tag);
	string i_tag;

	while( getline(iss,i_tag,'-') ){
		pos=-1;
		do{
			pos=body.find(i_tag,pos+1);
			if( pos!=string::npos && !isalnum(body[pos-1]) && !isalnum(body[pos+i_tag.size()]) )
				count++;
		}while(pos!=string::npos);
	}

	l.body_count[tag_number]=count;
}


//returns the index at which current tag occurs in question title, else returns string::npos
void question::processTitle(string current_tag){
	if( title.find(current_tag)==string::npos )
		l.tag_in_title[tag_number]=0;	
	else
		l.tag_in_title[tag_number]=1;
}

ios::pos_type searchFile(fstream &file,string token){
	file.clear();
	file.seekg(0,ios::beg);
	string line;
	string file_token;
	ios::pos_type current;
	
	while( file ){
		current=file.tellg();	
		getline(file,line);
		istringstream iss(line);
		getline(iss,file_token,' ');
		if( file_token==token )
			return current;
	}
	return -1;
}

/**** Class for training the program ******/

class train{
	fstream train_file;
	fstream tags_file;
	question q;

	public:
		
		train(char file_name[]){
			train_file.open( file_name,ios::in|ios::binary );
			tags_file.open( "tags_file.txt",ios::in|ios::out|ios::trunc|ios::binary );
			if( !train_file || !tags_file ){
				cout<<"Cannot open file"<<endl;
				exit(1);
			}
		}
		
		void beginTraining();
		void writeLearnings();
		void getQuestion();	
};

void train::beginTraining(){
	getQuestion();  //neglect file headings
	
	while(train_file){
	//for(int i=0;i<NUMBER;i++){
		getQuestion();
		q.learn();
		writeLearnings();
	}
	train_file.close();
	tags_file.close();
	
}


void train::getQuestion(){
	string s,t;
	char ch;
	
	for(int i=0;i<4;i++){
	  getline(train_file,s,'"');
	  getline(train_file,s,'"');

	  if(i!=3){	
		train_file.get(ch);
		while(ch!=',' && train_file){
			getline(train_file,t,'"');
			s = s+t;
			train_file.get(ch);
		}
	  }	  
		
	  switch(i){
		case 0: q.setId(atol( s.c_str() )); 
		  		break;
		case 1: transform(s.begin(), s.end(), s.begin(), ::tolower);
		  		q.setTitle(s);
		  		break;
        case 2: transform(s.begin(), s.end(), s.begin(), ::tolower); 
			  	q.setBody(s); 
		  		break;
		case 3:	transform(s.begin(), s.end(), s.begin(), ::tolower); 
			  	q.setTagList(s); 
		  		break;
	  }
	}
}


//writes the counts in the tags_file
void train::writeLearnings(){
	istringstream iss(q.getTagList());
	string token;
	int index;

	learnt l= q.getLearnings();	

	ios::pos_type pos;
	index=0;
	while( getline(iss,token,' ') ){
		pos= ::searchFile(tags_file,token);

		if( pos==(ios::pos_type)-1 ){
		//insert new token in the tags_file
			tags_file.clear();
			tags_file.seekp(0,ios::end);
			tags_file.width(40);
			tags_file.setf(ios::left);
			tags_file<<token;
			tags_file.width(20);
			tags_file<<l.tag_in_title[index];
			tags_file.width(20);
			tags_file<<l.body_count[index];
			tags_file.width(20);
			tags_file<<1<<endl;
		}
		else{
		//add findings of this token to previous entry of the token
			int in_title;
			int tag_occurrence;
			int tag_count;
			tags_file.clear();
			tags_file.seekg(pos,ios::beg);
			tags_file>>token>>in_title>>tag_occurrence>>tag_count;
			
			if(l.tag_in_title[index])
				in_title++;
			tag_occurrence+= l.body_count[index];
			tag_count++;

			tags_file.seekp(pos,ios::beg);
			tags_file.setf(ios::left);
			tags_file.width(40);
			tags_file<<token;
			tags_file.width(20);
			tags_file<<in_title;
			tags_file.width(20);
			tags_file<<tag_occurrence;
			tags_file.width(20);
			tags_file<<tag_count<<endl;
		}
		index++;
	}
}

class test{
	fstream test_file;
	fstream tags_file;
	fstream result_file;
	question q;

	public:
		test(char file[]){
			test_file.open(file,ios::in);
			tags_file.open( "tags_file.txt", ios::in);
			result_file.open("result.txt",ios::out);
			if( !test_file || !result_file ){
				cerr<<"Error opening test or result file"<<endl;
				exit(1);
			}
		};
		void beginTest();
		void predictTags();
		void getQuestion();	
};
/*
void test::beginTest(){
	cout<<"Begin Test dude";
}*/


void test::beginTest(){	
	getQuestion();
	result_file<<"Id,\"Tags\""<<endl;
	//while(test_file){
	for(int i=0;i<PREDICT;i++){	
		getQuestion();
		predictTags();
	}
}

void test::getQuestion(){
	string s,t;
	char ch;
		
	for(int i=0;i<3;i++){
	  getline(test_file,s,'"');
	  getline(test_file,s,'"');

	  
		if(test_file)
			test_file.get(ch);
		while(ch=='"' && test_file){
			getline(test_file,t,'"');
			s = s+t;
			test_file.get(ch);
		}
	  
		
	  switch(i){
		case 0: q.setId(atol( s.c_str() )); 
		  		break;
		case 1: transform(s.begin(), s.end(), s.begin(), ::tolower);
		  		q.setTitle(s);
		  		break;
        case 2: transform(s.begin(), s.end(), s.begin(), ::tolower); 
			  	q.setBody(s); 
		  		break;
	  }
	}
}

void test::predictTags(){
	string word;
	string text(q.getTitle() +" "+ q.getBody());
	ios::pos_type p;
	string predicted_tags("");
	stringstream ss(text);
	vector<string> all_words;
	
	
	while(ss >> word)
		all_words.push_back(word);
	sort(all_words.begin(), all_words.end());
    all_words.erase(std::unique(all_words.begin(), all_words.end()), all_words.end());

	for(unsigned i=0;i<all_words.size();i++){
				
			p= searchFile(tags_file,all_words[i]);

			if( p!=(ios::pos_type)-1 ){
				int word_count=0;
				int pos=-1;
				string tag;
				long in_title,tag_occurrence,tag_count,criteria;
				do{
					pos=text.find(all_words[i],pos+1);
					if( pos!=string::npos && !isalnum(text[pos-1]) && !isalnum(text[pos+all_words[i].size()]) ){
						word_count++;
					}
				}while(pos!=string::npos);
						
				tags_file.clear();
				tags_file.seekg(p,ios::beg);
				tags_file>>tag>>in_title>>tag_occurrence>>tag_count;
				criteria= (long)((in_title+ tag_occurrence)/tag_count);
				if( criteria && word_count >= criteria ){
					predicted_tags += all_words[i] + " ";
				}	
			}
	
	}
	result_file<<q.getId()<<",";
	result_file<<"\""<<predicted_tags<<"\""<<endl;
}

int main( int argc, char* argv[] ){

	if( argc!=3 ){
		cout<<"Usage: program train_file test_file"<<endl;
		return 1;
	}
	
	train t(argv[1]);
	t.beginTraining();

	test tt(argv[2]);
	tt.beginTest();
	
}