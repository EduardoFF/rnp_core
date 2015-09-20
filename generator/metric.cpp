/*
 * =====================================================================================
 *
 *       Filename:  metric.cpp
 *
 *    Description:  Calculates Metrics for Model
 *
 *        Version:  1.0
 *        Created:  11/12/2010 09:53:45 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  EDUARDO FEO 
 *        Company:  
 *
 * =====================================================================================
 */


#include "main.h"
#include "generator.h"
#include "Plotter.h"
#include "graph.h"
#include "network.h"
#include "metric.h"

extern param prob;

extern void calc_metric(vector< pair<double, double> > &net, vector<int> &mask_send, vector<int> &mask_rcv, vector<int> &mask_int,map< pair< int, int>, double> &ret_val,
			double _demand, double _sigma, double _ple, double _ttx	);

ostream &
operator<<(ostream &os, const Metric &m)
{
  m.stats(os);
  return os;
}

void
Metric::stats(ostream &os ) const
{
  os << "Metric Stats= max: " << max() 
    << " avg: " << average() << std::endl;
}

EstPRR_Metric::EstPRR_Metric(Network *net) : 
  Metric(net)
{
  vector< pair<double, double> > nodes;
  map< int, string> nodes_to_net;
  map<pair<int, int>, double> values;
  vector<int> mask_send, mask_rcv,mask_int;
  if( prob.lambda == 0){
    max_cost = 1.0;
    return;
  }
  for(int i = 0; i<net->numStatic();i++){
    Node n = net->getStatic(i);
    mask_send.push_back(nodes.size());
    mask_rcv.push_back(nodes.size());
    mask_int.push_back(nodes.size());
    nodes_to_net[nodes.size()]=n.id;
    nodes.push_back(make_pair(n.x,n.y));
  }
  for(int i =0;i<net->numBases();i++){
    Node n = net->getBase(i);
    mask_rcv.push_back(nodes.size());
    nodes_to_net[nodes.size()]=n.id;

    nodes.push_back(make_pair(n.x,n.y));

  }
  for(int i =0;i<net->numRelays();i++){
    Node n = net->getRelay(i);
    mask_send.push_back(nodes.size());
    mask_rcv.push_back(nodes.size());
    nodes_to_net[nodes.size()]=n.id;

    nodes.push_back(make_pair(n.x,n.y));
  }
  cout << "Ready to calc metric " 
    << " demand " << prob.prr_par.demand
    << " sigma " << prob.prr_par.sigma
    << " ttx " << prob.prr_par.ttx
    << "ple " << prob.prr_par.ple << endl;
  calc_metric(nodes,mask_send,mask_rcv,mask_int,values,
	      prob.prr_par.demand,
	      prob.prr_par.sigma,
	      prob.prr_par.ple,
	      prob.prr_par.ttx);
  cout << "Finished calculating metric..." << endl;
  map< pair<int, int>, double>::iterator it;
  double max_metric = 0.0;
  double min_metric = 1.0;


  for(it = values.begin();it != values.end();it++){
    double prr = it->second;
    max_metric = fmax(max_metric,prr);
    min_metric = fmin(min_metric,prr);
  }
  max_cost = -1.0;
  for(it = values.begin();it != values.end();it++){
    int s = (it->first).first;
    int r = (it->first).second;
    double prr = it->second;
    prr = (prr - min_metric)/(max_metric - min_metric);
    string sid = nodes_to_net[s];
    string rid = nodes_to_net[r];
    double cost;
    if(prob.lambda < 0)
      cost = (1 - prr);
    else{
      cost = 1 + prob.lambda*(1-prr);
    }
    max_cost = fmax(max_cost,cost);
    metric[make_pair(sid,rid)] = cost;
  }



}

double 
EstPRR_Metric::average() const
{
  return (1+prob.lambda + 1)/2.0;
}

double 
EstPRR_Metric::max() const
{
  return max_cost;
}

double 
EstPRR_Metric::operator()(string s, string r)
{
  map< pair<string, string>, double>::iterator it;
  it = metric.find(make_pair(s,r));
  if( it == metric.end()){
    //cout << s << " " << r << " linkcost not found - returning 1" << endl;
    return 1.0;
  }else{
    return it->second;
  }
}



DistSQ_Metric::DistSQ_Metric(Network *net): 
  Metric(net)
{
  vector< pair<double, double> > nodes;
  map< int, string> nodes_to_net;
  map<pair<int, int>, double> values;

  pair<double, double> mp;
  for(int i = 0; i<net->numStatic();i++)
  {
    Node n = net->getStatic(i);
    nodes_to_net[nodes.size()]=n.id;
    mp.first = n.x;
    mp.second = n.y;
    nodes.push_back(mp);
  }
  for(int i =0;i<net->numBases();i++)
  {
    Node n = net->getBase(i);
    nodes_to_net[nodes.size()]=n.id;
    mp.first = n.x;
    mp.second = n.y;
    nodes.push_back(mp);

  }
  for(int i =0;i<net->numRelays();i++)
  {
    Node n = net->getRelay(i);
    nodes_to_net[nodes.size()]=n.id;
    mp.first = n.x;
    mp.second = n.y;
    nodes.push_back(mp);

  }
  if(prob.verbose)
  {
    cout << "Ready to calc metric " 
      << " TTX RANGE " << prob.tx_range << endl;
  }
  double max_metric = 0.0;
  double min_metric = 1.0;


  for(int i=0; i < nodes.size(); i++)
  {
    for(int j=i+1; j < nodes.size(); j++)
    {
      double dx = nodes[i].first - nodes[j].first;
      double dy = nodes[i].second - nodes[j].second;
      double dist = sqrt(dx*dx + dy*dy);
      /// Here was a motherfucking BUG that took me several hours (days?) 
      /// to spot! - division by zero (dist=0) was causing CPLEX to hang!!!!!
      /// TO REMEMBER FOREVER AND EVER AND EVER -> -> IF CPLEX HANGS, CHECK FOR NAN !!
      dist = fmax(dist, EPSILON);
      values[make_pair(i,j)] = 1/(dist*dist);

      values[make_pair(j,i)] = 1/(dist*dist);
    }
  }

  map< pair<int, int>, double>::iterator it;
  for(it = values.begin();it != values.end();it++){
    double cost = it->second;
    max_metric = fmax(max_metric,cost);
    min_metric = fmin(min_metric,cost);
  }
  max_cost = -1.0;
  for(it = values.begin();it != values.end();it++){
    int s = (it->first).first;
    int r = (it->first).second;
    double met = it->second;
    met = (met - min_metric)/(max_metric - min_metric);
    string sid = nodes_to_net[s];
    string rid = nodes_to_net[r];
    double cost;
    if(prob.lambda < 0)
      cost = (1 - met);
    else{
      cost = 1 + prob.lambda*(1-met);
    }
    max_cost = fmax(max_cost,cost);
    metric[make_pair(sid,rid)] = cost;
  }



}


double 
DistSQ_Metric::average() const
{
  return (1+prob.lambda + 1)/2.0;
}

double 
DistSQ_Metric::max() const
{
  return max_cost;
}

double 
DistSQ_Metric::operator()(string s, string r)
{
  map< pair<string, string>, double>::iterator it;
  it = metric.find(make_pair(s,r));
  if( it == metric.end())
  {
    fprintf(stderr, "linkcost not found for %s %s - returning 1\n",
	    s.c_str(), r.c_str());
    return 1.0;
  }
  else
  {
    //TODO round up to avoid numerical inestabilities?
    //cout << s << " - " << r << " " << it->second << endl;
    return it->second;
  }

}

LinkWeight_Metric::LinkWeight_Metric(Network *net) : 
  Metric(net)
{
  map<pair<string, string>, double> values;

  printf("Creating LinkWeight metric\n");
  double max_metric = 0.0;
  double min_metric = 1.0;
  printf("Network has %d links\n", 
	 net->links.size());


  for(int i=0; i < net->size(); i++)
  {
    Node &ni = net->getNode(i);
    for(int j=0; j < net->size(); j++)
    {
      if( i == j)
	continue;
      Node &nj = net->getNode(j);
      if( net->areLinked(ni,nj))
      {
	double w = net->getLinkWeight(ni,nj);
	values[make_pair(ni.id,nj.id)] = w;
	//printf("%s -> %s = %f\n", 
	       //ni.id.c_str(),nj.id.c_str(), w);

      }
    }
  }

  ITERATOR(values) it;
  for(it = values.begin();it != values.end();it++){
    double cost = it->second;
    max_metric = fmax(max_metric,cost);
    min_metric = fmin(min_metric,cost);
  }
  printf("max_metric %f min_metric %f\n", 
	 max_metric, min_metric);
  max_cost = -1.0;
  for(it = values.begin();it != values.end();it++){
    string sid = (it->first).first;
    string tid = (it->first).second;
    double met = it->second;
    if( fabs(max_metric -min_metric) > EPSILON )
    {
    	met = (met - min_metric)/(max_metric - min_metric);
    }
    double cost;
    if(prob.lambda < 0)
      cost = (1 - met);
    else{
      cost = 1 + prob.lambda*(1-met);
    }
    //printf("Setting link cost .%s. .%s. %f\n",
	   //sid.c_str(), tid.c_str(), cost);
    max_cost = fmax(max_cost,cost);
    metric[make_pair(sid,tid)] = cost;
  }
  printf("max_cost %f\n", max_cost);
  //max_cost = -1.0;
}

double 
LinkWeight_Metric::average() const
{
  return (1+prob.lambda + 1)/2.0;
}

double 
LinkWeight_Metric::max() const
{
  return max_cost;
}
double LinkWeight_Metric::operator()(string s, string r){
  map< pair<string, string>, double>::iterator it;
  it = metric.find(make_pair(s,r));
  if( it == metric.end()) {
    printf("Link cost .%s. .%s. not found\n",
	   s.c_str(), r.c_str());
    //cout << s << " " << r << " linkcost not found - returning 1" << endl;
    return 1.0;
  }else{
    return it->second;
  }

}

