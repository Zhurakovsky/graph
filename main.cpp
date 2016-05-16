#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <queue>

using namespace std;

void deikstra(int i, vector< vector< pair<int, int> > > &gr, vector<vector<int> > &dist, vector<vector<int > > &pathes);
void buildWorkRoute(vector<int> &routeOrder, vector<vector<int > >  &pathes, vector<int> &routeWork);
void fillSubroute(int pointFrom, int pointTo, vector<int> &subRoute, vector<vector<int > >  &pathes);
int getRouteDuration(bool &contFlag, int i, vector<int> &routeWork, vector<vector<int> > &dist, vector<vector<vector<int> > > &weighters);

int main() {
    //исходный граф
    vector< vector< pair<int, int> > > graph(23, vector<pair<int, int> >());
    //вектор расстояний до каждой точки
    vector<vector<int> > distances(23, vector<int>(23, 9999));
    //вектор путей до каждой точки
    vector<vector<int > > pathes(23, vector<int >(23, -1));
    //вектор дополнительных весов для сегментов графа
    vector<vector<vector<int> > > weighters(23, vector<vector<int> >(23, vector<int>(1440, 0)));
    //вектор заказа проезда по маршруту с ключевыми точками
    vector<int> routeOrder;
    //вектор финального проезда по маршруту со всеми промежуточнымии точками
    vector<int> routeWork;
    //вектор стабильных периодов для старта маршрута
    vector<pair<int, int> > periods;
    int v, e;
    ifstream fin;
    
    //Открываем и считываем основной граф
    fin.open("in.txt", ifstream::in);
    if (!fin.is_open()){
        cout << "Unable to open file in.txt\n";
    }
    fin >> v;
    fin >> e;
    while(fin) {
        int v1, v2, cost;
        fin >> v1 >> v2 >> cost;
        graph[v1].push_back(make_pair(cost, v2));
    }
    fin.close();
    
    //Открываем и считываем корректировки к графу
    ifstream finCorrections;
    finCorrections.open("corrections.txt", ifstream::in);
    if (!finCorrections.is_open()){
        cout << "Unable to open file corrections.txt\n";
    }
    while(finCorrections) {
        int pointFrom, pointTo, periodFrom, periodTo, increaser;
        finCorrections >> pointFrom >> pointTo >> periodFrom >> periodTo >> increaser;
        vector<int>::iterator it;
        for (it = weighters[pointFrom][pointTo].begin()+periodFrom; it != weighters[pointFrom][pointTo].begin()+periodTo+1; it++) {
            *it = increaser;
        }
    }
    finCorrections.close();
    for (int i = 0; i < (int)graph.size(); i++) {
        deikstra(i, graph, distances, pathes);
    }
    
    //Открываем и считываем маршрут движения
    ifstream finRoute;
    finRoute.open("route.txt", ifstream::in);
    if (!finRoute.is_open()){
        cout << "Unable to open file route.txt\n";
    }
    while(finRoute) {
        int tmpValue;
        finRoute >> tmpValue;
        routeOrder.push_back(tmpValue);
    }
    finRoute.close();
    buildWorkRoute(routeOrder, pathes, routeWork);
    cout << "Work path\n";
    vector<int>::iterator itWorkPath;
    for ( itWorkPath = routeWork.begin(); itWorkPath != routeWork.end(); itWorkPath++ ) {
        cout << *itWorkPath << " ";    
    }
    cout << endl << endl;
    
    bool contFlag = true;
    int startPeriod = 0;
    int endPeriod = 0;
    for (int i = 0; i < 1440; i += 60) {
        int hours = i / 60;
        int routeDuration = getRouteDuration(contFlag, i, routeWork, distances, weighters);
        cout << "Start of route: " << hours << " hours. Duration of route: ";
        cout << routeDuration / 60 << " hours " << routeDuration % 60 << " minutes." << endl;
        if (!contFlag && startPeriod != -999) {
            endPeriod = i - 60;
            periods.push_back(make_pair(startPeriod, endPeriod));
            startPeriod = -999;
            contFlag = true;
        } else if (contFlag && startPeriod == -999 ) {
            startPeriod = i + 60;
        } else if (contFlag && i == 1380 ) {
            endPeriod = i;
            periods.push_back(make_pair(startPeriod, endPeriod));
        } else if (!contFlag && startPeriod == -999) {
            contFlag = true;
        }
    }
    vector<pair<int, int> >::iterator itPeriods;
    cout << endl;
    cout << "Stable periods for route start:" << endl;
    for (itPeriods = periods.begin(); itPeriods != periods.end(); itPeriods++ ) {
        pair<int, int> tmpPair = *itPeriods;
        cout << "From " << tmpPair.first / 60 << " hours to " << tmpPair.second / 60 << " hours." << endl; 
    }
	getchar();
	return 0;
}

void deikstra(int i, vector< vector< pair<int, int> > > &gr, vector<vector<int> > &dist, vector<vector<int > > &pathes) {
    if (gr[i].size() == 0) {
        vector<int>::iterator it;
        for (it = dist[i].begin(); it != dist[i].end(); it++) {
            *it = (-1);
        }
        return;
    }
    dist[i][i] = 0;
    pathes[i][i] = 0;
    queue<int> que;
    vector<bool> visited(23, false);
    que.push(i);
    
    while(!que.empty()) {
        int v = que.front();
        
        que.pop();
        for(int j = 0; j < (int)gr[v].size(); j++) {
            pair<int, int> para(gr[v][j]);
            int para_v = para.second;
            int para_cost = para.first;
            if(dist[i][v] + para_cost < dist[i][para_v]) {
                dist[i][para_v] = dist[i][v] + para_cost;
                pathes[i][para_v] = v;
                if(!visited[para_v]) {
                    que.push(para_v);
                    visited[v] = true;
                }
            }
        }
    }
}

void buildWorkRoute(vector<int> &routeOrder, vector<vector<int > >  &pathes, vector<int> &routeWork) {
    vector<int>::iterator it1 = routeOrder.begin();
    vector<int>::iterator it2 = routeOrder.begin()+1;
    vector<int>::iterator it3 = routeOrder.begin();
    routeWork.push_back((int)*it3);
    for(;it2 != routeOrder.end(); it1++, it2++) {
        int pointFrom = *it1;
        int pointTo = *it2;
        vector<int> subRoute;
        
        fillSubroute(pointFrom, pointTo, subRoute, pathes);
        routeWork.insert(routeWork.end(), subRoute.begin(), subRoute.end());
    }
}

void fillSubroute(int pointFrom, int pointTo, vector<int> &subRoute, vector<vector<int > >  &pathes) {
    while(pointFrom != pointTo) {
        subRoute.insert(subRoute.begin(), pointTo);
        pointTo = pathes[pointFrom][pointTo];
    }
}

int getRouteDuration(bool &contFlag, int startTime, vector<int> &routeWork, vector<vector<int> > &dist, vector<vector<vector<int> > > &weighters) {
    int resultDuration = 0;
    int subTime = startTime;
    vector<int>::iterator it1 = routeWork.begin();
    vector<int>::iterator it2 = routeWork.begin()+1;
    for(; it2 != routeWork.end(); it1++, it2++) {
        int pointFrom = *it1;
        int pointTo = *it2;
        int sumOfDurations = 0;

        sumOfDurations += dist[pointFrom][pointTo];
        sumOfDurations += weighters[pointFrom][pointTo][subTime];
        resultDuration += sumOfDurations;
        subTime += sumOfDurations;
        if ( weighters[pointFrom][pointTo][subTime] > 0  && contFlag) {
                contFlag = false;
        }
        if (subTime >= 1440) {
            subTime %= 1440;
        }
    }
    return resultDuration;
}