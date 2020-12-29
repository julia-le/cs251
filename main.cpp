/*main.cpp*/

// Name: Thu Le
// Covid-19 data analysis program
// Analyze data from the daily reports and world-facts
	 // of country having covid 19

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <experimental/filesystem>
#include <locale>
#include <iomanip>
#include <vector>
#include <algorithm>    // std::sort
#include <map>

using namespace std;
namespace fs = std::experimental::filesystem;

// Structs for maping 
	struct DayByDay{
		string country;
		string date;
		int confirmed;
		int death;
		int recovered;
	};

	struct Info{
		int population;
		double lifeExpectancy;
		vector <DayByDay> DayByDayData;
	};

	struct timeLineData{ // For CountryName function
		string date;
		int numberOfDay;
		int value;
	};
	
//
// getFilesWithinFolder
//
// Given the path to a folder, e.g. "./daily_reports/", returns 
// a vector containing the full pathnames of all regular files
// in this folder.  If the folder is empty, the vector is empty.
//
vector<string> getFilesWithinFolder(string folderPath)
{
  vector<string> files;

  for (const auto& entry : fs::directory_iterator(folderPath))
  {
    files.push_back(entry.path().string());
  }

	sort(files.begin(), files.end());
	

  return files;
}

// If the user press help
void Help(){
	cout<<"Available commands: "<<endl;
	cout<<" <name>: enter a country name such as Us or China"<<endl;
	cout<<" countries: list all countries and most recent report"<<endl;
	cout<<" top10: list of top 10 countries based on most recent # of confirmed cases "<<endl;
	cout<<" totals: world-wide totals of confirmed, deaths, recovered "<<endl;
	cout<<" prevalence: Percentage of residents having the virus over country population "<<endl;
}

// Read and store info from fact files 
map<string, string> GetFact(string factFile){
	map<string, string> map;
	
	ifstream inFile;
	// Open the files
	inFile.open(factFile);
	
	// Check if the file opened
	if (!inFile.is_open()){
		cout<<" ** Fail to open '"<<factFile<<"' "<<endl;
	}
	
	// Store values to map
	string line;
	getline(inFile, line); // Skip the first line
	while (getline(inFile, line)){
		stringstream s(line);
		string number, country, data;
		
		// Spare the line into strings 
		getline(s, number, ',');
		getline(s, country, ',');
		getline(s, data, ',');
		
		// Store value into map
		map[country]=data;
	}
	return map;
} 


// Parse line into categories 
void Parse(string line, string & country, int & confirmed, int & death, int & recovered){
	if (line[0]=='"'){
		line.erase(0,1); // Delete the leading "
		size_t pos=line.find(','); // Find embedded ','
		line.erase(pos,1); // Delete ','
		pos = line.find('"'); // Find closing "
		line.erase(pos,1); // Delete closing
	}
	// Parese the information 
	string province, lastUpdate, confirmedS, deathS, recoveredS;
	stringstream s(line);
	getline(s, province, ',');
	getline(s, country, ',');
	getline(s, lastUpdate, ',');
	getline(s, confirmedS, ',');
	getline(s, deathS, ',');
	getline(s, recoveredS, ',' );
	
	// Deal with special data
	if (confirmedS==""){
		confirmedS="0";
	}
	if (deathS==""){
		deathS="0";
	}
	if (recoveredS==""){
		recoveredS="0";
	}
	if (country=="Mainland China"){
		country="China";
	}
	if (country=="Republic of Korea"){
		country="South Korea";
	}
	
	// Convert string to interger 
	confirmed=stoi(confirmedS);
	death=stoi(deathS);
	recovered=stoi(recoveredS);
	
}

// Go through the daily reports and store info 
map<string, vector<DayByDay>> GetDailyReport(vector<string>dailyFile){
	map <string, vector<DayByDay>> data;
	for (auto i:dailyFile){
		ifstream inFile;
		// Open the files
		inFile.open(i);

		// Check if the file opened
		if (!inFile.is_open()){
			cout<<" ** Fail to open '"<<i<<"' "<<endl;
		}
		
		// Parse the date from the file name
		string temp = i;
		temp.erase(temp.begin(), temp.begin()+16);
		string dateFile="";
		stringstream s(temp);
		getline(s, dateFile, '.');
		
		// Collect the data
		string line;
		getline(inFile, line);
		while(getline(inFile, line)){
			string country = " ";
			int confirmed=0;
			int death = 0;
			int recovered = 0;
			
			// Parse data
			Parse(line, country, confirmed, death, recovered);
			
			// Insert to map
				// Check if the date already exist on that day
				auto iter=data.find(country);
				if ((iter!=data.end())&& ((iter->second).back().date==dateFile)){
					data[country].back().confirmed += confirmed;
					data[country].back().death+= death;
					data[country].back().recovered += recovered;
				}
				else{
					// Assign the struct values
					DayByDay dateData;
					dateData.country=country;
					dateData.date=dateFile;
					dateData.confirmed=confirmed;
					dateData.death=death;
					dateData.recovered=recovered;
					// Insert to the map
					data[country].push_back(dateData);
				}
				
	  }// End of while 

	}// End of for
	return data;
}

// Combine all the maps into a big map - Merge maps
map<string,Info> CombineData(map<string, string> lifeVal, 
							 map<string, string> population, 
							 map<string, vector<DayByDay>> dailyReport){
	map<string, Info> data;
	for(pair<string, vector<DayByDay>> P:dailyReport){
		Info temp;
		// Insert population
		auto iterP=population.find(P.first);
			if (iterP==population.end()){// No population fact
				temp.population=0;
			}
			else {
				temp.population=stoi(iterP->second);
			}
			
			// Insert lifeExpectancy
		       iterP=lifeVal.find(P.first);
			if (iterP==lifeVal.end()){// No life expectancy fact
				temp.lifeExpectancy=0;
			}
			else {
				temp.lifeExpectancy=stod(lifeVal[P.first]);
			}
			
			// Insert daily report 
			temp.DayByDayData=P.second;
			
			// Store all the info of a country into a big map
			data[P.first]=temp;
		
	}
	return data;
	
}

// Function for "countries" command -> Present the latest update of all countries
void Countries(map<string, Info> data, string lastDate){
	// Present the data
	for(pair<string, Info> P:data){
		// Go through the map
		if (P.second.DayByDayData.back().date!=lastDate){// When the countries is unified with others
				cout<< P.first<<": 0, 0, 0"<<endl;// ex: Aruba
		}
		else {// For all other normal countries
			cout<< P.first<<": "<<P.second.DayByDayData.back().confirmed
				<<", "<<P.second.DayByDayData.back().death
				<<", "<<P.second.DayByDayData.back().recovered<<endl;
		}
	}
}

// Compare function for build-in sort for Top10 function 
// -> sort according to confirmed cases in the struct
bool Compare(DayByDay left, DayByDay right){
	return left.confirmed<right.confirmed;
}

// Store latest report values for totals function
vector<DayByDay> LatestReport(map <string, Info> data, string lastDate){
	DayByDay temp; // Value to store data of countries in the lattest report 
	vector<DayByDay> list;
	int i=0;
	for (pair<string, Info> P:data){
		i++;
		temp=(P.second.DayByDayData.back());
		if (temp.date==lastDate)
			list.push_back(temp);
	}
	
	return list;
}

// Totals function for "totals" command 
// -> print out totals confirmed, death, recovered cases in the world
void Totals(map<string, Info> data, string lastDate){
	// Get the latest report list
	vector<DayByDay> list=LatestReport(data, lastDate);
	
	//Get the latest dateData
    string date=list[0].date;
	// Get the totals of confirmed, death, recovered
		// Initialize the values
	int totalConfirmed=0;
	int totalDeath=0;
	int totalRecovered=0;
		// Take the values from the report 
	for (int i=0; i<list.size(); i++){
		totalConfirmed += list[i].confirmed;
		totalRecovered += list[i].recovered;
		totalDeath += list[i].death;
	}
	
	// Calculate the rates
	double deathRate =double(totalDeath)/totalConfirmed*100;
	double recoveredRate = double(totalRecovered)/totalConfirmed*100;
	
	// Present the data
	cout<<"As of "<<date<<", the world-wide totals are:"<<endl;
	cout<<" confirmed: "<<totalConfirmed<<endl;
	cout<<" deaths: "<<totalDeath<<" ("<<deathRate<<"%)"<<endl;
	cout<<" recovered: "<<totalRecovered<<" ("<<recoveredRate<<"%)"<<endl;	
	
}

// Function for "top 10" command
// -> Print out top 10 countries having the most cases
void Top10(map <string, Info> data, string lastDate){
	vector<DayByDay> list;
	// Get the latest report data
	list=LatestReport(data, lastDate);
	// Sort data
	sort(list.begin(), list.end(), Compare);
	
// 	// Present data
	int count=0; // The order of the countries
	for(int i=list.size()-1; i>=(list.size()-10); i--){
		count++;
		cout<<count<<". "<<list[i].country<<": "<<list[i].confirmed<<endl;
	}
}



// Find the date of first death for the CountriesName function
string FirstDeath(Info data){
	for (int i=0; i<data.DayByDayData.size(); i++){
		if (data.DayByDayData[i].death != 0){
			return data.DayByDayData[i].date;
		}
	}
	return "none";
}

// Print timeline function for CountriesName function
void PrintTimeline(vector<timeLineData>data){
	if(data.size()<=14){
		for(int i=0; i<data.size(); i++){
			cout<<data[i].date<<" (day "<<data[i].numberOfDay<<"): "<<data[i].value<<endl;
		}
	}
	else {
		for(int i=0; i<7; i++){
			cout<<data[i].date<<" (day "<<data[i].numberOfDay<<"): "<<data[i].value<<endl;
		}
		cout<<" ."<<endl;
		cout<<" ."<<endl;
		cout<<" ."<<endl;
		for (int i=data.size()-7; i<data.size(); i++){
			cout<<data[i].date<<" (day "<<data[i].numberOfDay<<"): "<<data[i].value<<endl;
		}
	}
}

// 
// Countries's name option
void CountriesName(Info data){
	// Present facts
	cout<<"Population: "<< data.population<<endl;
	cout<<"Life expectancy: "<< data.lifeExpectancy<<" years"<<endl;
	
	// Present latest data
	cout<<"Latest data:"<<endl;
	cout<<" confirmed: "<< data.DayByDayData.back().confirmed<<endl;
	cout<<" deaths: "<< data.DayByDayData.back().death<<endl;
	cout<<" recovered: "<< data.DayByDayData.back().recovered<<endl;
	
	// Overal data
	cout<<"First confirmed case: "<<data.DayByDayData[0].date<<endl;
	
	// First recorded death
	cout<<"First recorded death: "<<FirstDeath(data)<<endl; 
	
	// Timeline
	cout<<"Do you want to see a timeline? Enter c/d/r/n> ";
	string option;
	getline(cin, option);
		if (option=="n")
			return;
		else if (option=="c"){
			vector<timeLineData> timeline;
			cout<<"Confirmed:"<<endl;
			// Find the first confirmed
			int it;
			for (it=0; it<data.DayByDayData.size(); it++){
				if (data.DayByDayData[it].confirmed != 0)
					break;
			}
			// Store timeline
			for (int i=it; i<data.DayByDayData.size(); i++){
					timeLineData temp;
					temp.date=data.DayByDayData[i].date;
					temp.value=data.DayByDayData[i].confirmed;
					temp.numberOfDay=i+1;
					timeline.push_back(temp);
			}
			// Print
			PrintTimeline(timeline);
		}
		else if (option=="d"){
			vector<timeLineData> timeline;
			cout<<"Deaths:"<<endl;
			// Find the first death
			int it;
			for (it=0; it<data.DayByDayData.size(); it++){
				if (data.DayByDayData[it].death != 0)
					break;
			}
			// Store timeline
			for (int i=it; i<data.DayByDayData.size(); i++){
					timeLineData temp;
					temp.date=data.DayByDayData[i].date;
					temp.value=data.DayByDayData[i].death;
					temp.numberOfDay=i+1;
					timeline.push_back(temp);
			}
			// Print
			PrintTimeline(timeline);
		}
		else if (option=="r"){
			vector<timeLineData> timeline;
			cout<<"Recovered:"<<endl;
			// Find the first recovered
			int it;
			for (it=0; it<data.DayByDayData.size(); it++){
				if (data.DayByDayData[it].recovered != 0)
					break;
			}
			// Store timeline
			for (int i=it; i<data.DayByDayData.size(); i++){
					timeLineData temp;
					temp.date=data.DayByDayData[i].date;
					temp.value=data.DayByDayData[i].recovered;
					temp.numberOfDay=i+1;
					timeline.push_back(temp);
			}
			// Print
			PrintTimeline(timeline);
		}
}

//*** Student's option ***//
// Get prevalence function (percentage of confirmed cases/population)
void GetPrevalence(Info data){
	int confirmed = data.DayByDayData.back().confirmed;
	int population = data.population;
	float prevalence = float(confirmed)/population*100;
	cout<<"Prevalence = confirmed / population * 100 = "<<prevalence<<"%"<<endl;
}

//
// main:
//
int main()
{
  cout << "** COVID-19 Data Analysis **" << endl;
  cout << endl;
  cout << "Based on data made available by John Hopkins University" << endl;
  cout << "https://github.com/CSSEGISandData/COVID-19" << endl;
  cout << endl;

  //
  // setup cout to use thousands separator, and 2 decimal places:
  //
  cout.imbue(std::locale(""));
  cout << std::fixed;
  cout << std::setprecision(2);

  //
  // get a vector of the daily reports, one filename for each:
  //
  vector<string> dailyfiles = getFilesWithinFolder("./daily_reports/");
  vector<string> factfiles = getFilesWithinFolder("./worldfacts/");
	
	// Declare maps
	map<string, string> lifeExpectancyMap=GetFact(factfiles[0]);
	map<string, string> populationMap=GetFact(factfiles[1]); 
	map<string, vector<DayByDay>> dailydataMap=GetDailyReport(dailyfiles);
	map<string, Info > dataMap=CombineData(lifeExpectancyMap, populationMap, dailydataMap);
	
	// Print out overal info
    cout<<">> Processed "<< dailyfiles.size()<<" daily reports"<<endl;
    cout<<">> Processed "<< factfiles.size()<< " files of world facts"<<endl;
	cout<<">> Current data on "<<dailydataMap.size()<<" countries"<<endl;
	
	// Get the lattest date
	string tempS=dailyfiles.back();
	tempS.erase(tempS.begin(), tempS.begin()+16);
	string lastDate="";
	stringstream s(tempS);
	getline(s, lastDate, '.');
	
	// Start the program
	string option;
	cout<<endl;
	
		// Take user input
	cout<<"Enter command (help for list, # to quit)> ";
		getline(cin,option);
	
		// Go through options
	while (option!="#"){
		auto iter = dataMap.find(option); // For <CountriesName> option
		// Help option
		if (option=="help"){
			Help();
		}
		// Countries option
		else if (option=="countries"){
			Countries(dataMap,lastDate);
		}
		// Top 10 option
		else if (option=="top10"){
			Top10(dataMap, lastDate);
		}
		// Total option
		else if (option=="totals"){
			Totals(dataMap,lastDate);
		}
		// Prevalence option
		else if (option=="prevalence"){
				// Identify which country to calculate
			cout<<"Which country you want to calculate? ";
			string country;
		    getline(cin,country);
				// Check if country exist
			iter = dataMap.find(country);
			if (iter!=dataMap.end()){
				GetPrevalence(iter->second);// Calculating function
			}
			else cout<<"country not found ..."<<endl;
		}
		// <Countries name option>
		else if (iter!=dataMap.end()){
				CountriesName(iter->second);
		}
		else {// Unidentifed command
			cout<<"country or command not found...";
		}
		
		// Ask for new option
		cout<<endl;
		cout<<"Enter command> ";
		getline(cin, option);
	}
  cout<<endl;
  return 0;
}
