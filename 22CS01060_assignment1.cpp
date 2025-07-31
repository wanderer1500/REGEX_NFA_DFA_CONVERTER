#include<bits/stdc++.h>
using namespace std;
using NodePtr  = shared_ptr<class Node>;
int numNodes = 0;

class Node{
  public:
  int nodeId;
  bool isStart,isEnd;
  Node(bool isStart=false,bool isEnd=false){
    nodeId = numNodes++;
    this->isStart = isStart;
    this->isEnd = isEnd;
  }
 



};

 struct NodeCompare {
    bool operator()(const shared_ptr<Node>& a, const shared_ptr<Node>& b) const {
        return a->nodeId < b->nodeId;
    }
  };

class Machine{
  public:
  NodePtr startNode;
  NodePtr endNode;
  set<NodePtr,NodeCompare> endNodes;
  map<pair<NodePtr,NodePtr>,vector<char>> transition_edges;
  Machine(NodePtr startNode,NodePtr endNode){
    startNode->isStart = true;
    endNode->isEnd = true;
    this->startNode = startNode;
    endNodes.insert(endNode);
  }

  void addEdge(NodePtr from,NodePtr to,char transition_input){
     transition_edges[{from,to}].push_back(transition_input);

  }
  void addEndNode(NodePtr endNode){
    endNode->isEnd = true;
    endNodes.insert(endNode);
  }
  
  void PrintTransitions(){
    cout<<"Transitions: \n";
    for(auto& [key,val]:transition_edges){
      auto from = key.first;
      auto to = key.second;
      cout<<"From Node: "<<from->nodeId<<" To Node: "<<to->nodeId<<" inputs: ";
      for(auto c:val){
        cout<<c<<" ";
      }
      cout<<"\n";
    }

    cout<<"StartNodes: "<<startNode->nodeId<<"\n";
    cout<<"EndNodes: ";
    for(auto e:endNodes){
      cout<<e->nodeId<<" ";
    }
    cout<<"\n";

  }





};

Machine Union(Machine M1,Machine M2){
    auto S1 = M1.startNode;
    auto E1 = M1.endNodes;
    auto S2 = M2.startNode;
    auto E2 = M2.endNodes;
    auto S3 = make_shared<Node>(true, false);
    auto E3 = make_shared<Node>(false, true);
     
    Machine M3(S3, E3);

    M3.addEdge(S3, S1, 'e');
    M3.addEdge(S3, S2, 'e');
    for (auto& e : E1){
         M3.addEdge(e, E3, 'e');
    }
    for (auto& e : E2){
       M3.addEdge(e, E3, 'e');
    }
    M3.transition_edges.insert(M1.transition_edges.begin(), M1.transition_edges.end());
    M3.transition_edges.insert(M2.transition_edges.begin(), M2.transition_edges.end());


    return M3;

}

Machine Concatenate(Machine M1,Machine M2){
    auto S1 = M1.startNode;
    auto E1 = M1.endNodes;
    auto S2 = M2.startNode;
    auto E2 = M2.endNodes;
    auto S3 = make_shared<Node>(true, false);
    auto E3 = make_shared<Node>(false, true);

    Machine M3(S3, E3);

    M3.addEdge(S3,S1,'e');
    for (auto& e : E1){
         M3.addEdge(e, S2, 'e');
    }
    for (auto& e : E2){
       M3.addEdge(e, E3, 'e');
    }
    M3.transition_edges.insert(M1.transition_edges.begin(), M1.transition_edges.end());
    M3.transition_edges.insert(M2.transition_edges.begin(), M2.transition_edges.end());

    return M3;
}

Machine Closure(Machine M){
   auto S = make_shared<Node>(true,false);
   auto E = make_shared<Node>(false,true);
   Machine nM (S,E);
   nM.addEdge(S,M.startNode,'e');
   for (auto f : M.endNodes) {
        nM.addEdge(f, M.startNode, 'e');
        nM.addEdge(f, E, 'e');
    }
  nM.transition_edges.insert(M.transition_edges.begin(), M.transition_edges.end());

  return nM;
}


int precedence(char op){
  if(op=='*') return 3;
  if(op=='.') return 2;
  if(op=='+') return 1;

  return 0;
}

string toPostfix(string exp){
  string output="";
  stack<char> stk;

  for(auto c:exp){
    if(isalnum(c)) output+=c;
    else if(c=='(') stk.push(c);
    else if(c==')'){
      while(!stk.empty() &&  stk.top()!='('){
        output+=stk.top();
        stk.pop();
      }
      stk.pop();
    }else{
      while(!stk.empty() && precedence(stk.top())>=precedence(c)){
        output+=stk.top();
        stk.pop();
      }
      stk.push(c);
    }


  }
  while(!stk.empty()){
    output+=stk.top();
    stk.pop();
  }
  return output;

}

Machine toNFA(string postfix){
  stack<Machine> stk;
  for(auto c:postfix){
    if(isalnum(c)){
            auto start = make_shared<Node>(true, false);
            auto end = make_shared<Node>(false, true);
            Machine m(start, end);
            m.addEdge(start, end, c);
            stk.push(m);
    }
    else if(c=='*'){
            Machine m = stk.top(); stk.pop();
            stk.push(Closure(m));
    }
     else if (c == '+') {
            Machine m2 = stk.top(); stk.pop();
            Machine m1 = stk.top(); stk.pop();
            stk.push(Union(m1, m2));
        }
        else if (c == '.') {
            Machine m2 = stk.top(); stk.pop();
            Machine m1 = stk.top(); stk.pop();
            stk.push(Concatenate(m1, m2));
        }
  }
  return stk.top();
}

set<char> alphabet; 
NodePtr nfaStart;
using StateSet = set<NodePtr,NodeCompare>;
set<NodePtr,NodeCompare> nfaAcceptstates;
map<NodePtr, map<char, set<NodePtr>>> nfa_transitions;


StateSet epsilonClosure(const StateSet& states){
  stack<NodePtr> stk;
  StateSet closure = states;
  
for (auto s : states) stk.push(s);
  while(!stk.empty()){
    auto curr = stk.top();stk.pop();
    
    for(auto next :nfa_transitions[curr]['e']){
      if(closure.count(next)) continue;
      closure.insert(next);
      stk.push(next);
      
    }
  }

  return closure;

}


StateSet move( StateSet& states, char symbol) {
    StateSet result;
    for (auto s : states) {
        for (auto t : nfa_transitions[s][symbol]) {
            result.insert(t);
        }
    }
    return result;
}

void nfaTodfa(){
    map<StateSet, int> stateId;
    map<int, map<char, int>> dfaTransitions;
    set<int> dfaAccepting;
    queue<StateSet> q;
    StateSet startClosure = epsilonClosure({nfaStart});
    int idCounter = 0;
    stateId[startClosure] = idCounter++;

    q.push(startClosure);

    while(!q.empty()){
       StateSet current = q.front(); q.pop();
       int fromId = stateId[current];
       for (char ch : alphabet) {
            StateSet reached = epsilonClosure(move(current, ch));
            if (reached.empty()) continue;

            if (!stateId.count(reached)) {
                stateId[reached] = idCounter++;
                q.push(reached);
            }
            int toId = stateId[reached];
            dfaTransitions[fromId][ch] = toId;
        }

    }


      for (auto& [stateSet, id] : stateId) {
        for (auto n : stateSet) {
            if (nfaAcceptstates.count(n)) {
                dfaAccepting.insert(id);
                break;
            }
        }
    }

     cout << "DFA Transitions:\n";
    for (auto& [from, edges] : dfaTransitions) {
        for (auto& [ch, to] : edges) {
            cout << "From " << from << " --" << ch << "--> " << to << "\n";
        }
    }

    cout << "Start state: 0\n";
    cout << "Accepting states: ";
    for (int s : dfaAccepting) cout << s << " ";
    cout << "\n";
}


int main(){

  cout<<" Enter the regular expression [Note that . must be used for concatenation and e is reserved for epsilon ]: ";
   string exp;
   getline(cin >> ws, exp);

   string p = toPostfix(exp);
   Machine m = toNFA(p);
   m.PrintTransitions();
   nfaStart = m.startNode;
   nfaAcceptstates = m.endNodes;
   for(auto& [key,val]:m.transition_edges){
      for(auto input:val){
        auto from = key.first;
        auto to = key.second;
        if(input!='e') alphabet.insert(input);
        nfa_transitions[from][input].insert(to);
      }
      
   }
  //  cout<<nfa_transitions.size();

   nfaTodfa();
}