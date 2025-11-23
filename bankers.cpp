#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

void readInput(const string &filename,int &nProcesses,int &nResources,vector<vector<int>> &allocation,vector<vector<int>> &maxMatrix,vector<int> &available) {
ifstream fin(filename);

    if(!fin) 
    {
        cerr << "Error:Cannot open input file: " <<filename << endl;
        exit(1);

    }

    if(!(fin >> nProcesses >> nResources))
     {
        cerr<< "Error: Failed to read number of processes." <<  endl;
        exit(1);

    }

    allocation.assign(nProcesses, vector<int>(nResources,0));

    maxMatrix.assign(nProcesses,vector<int>(nResources,  0));

    available.assign(nResources,0);


    for(int i= 0; i < nProcesses;++i) 
    {
     for(int j =0; j < nResources; ++j) 
        {
    if(!(fin >>allocation[i][j])) 
     {
                cerr << "Error: Failed to read the allocation[" <<i<<"]["<< j  << "]." <<  endl;
                exit(1);
            }
        }
    }

    for(int i= 0; i < nProcesses; ++i) 
    {
        for(int j= 0; j <nResources;  ++j)
         {
    if(!(fin>> maxMatrix[i][j])) 
 {
    cerr <<"Error: Failed to read max["<<i<<"]["<<j << "]." << endl;

    exit(1);
            }
        }
    }

    for(int j= 0; j< nResources;++j)
     {
        if(!(fin >> available[j])) 
        {
     cerr << "Error: Failed to read available[" << j << "]." << endl;
         exit(1);
        }
    }

    fin.close();
}

vector<vector<int>>computeNeed(const vector<vector<int>> &maxMatrix,const vector<vector<int>> &allocation) 
{
    int nProcesses =maxMatrix.size();
    int nResources= maxMatrix[0].size();
    vector<vector<int>> need(nProcesses,vector<int>(nResources,0));

    for (int i =0; i < nProcesses;++i) 
    {
    for (int j = 0;j < nResources;++j) {
        need[i][j] =maxMatrix[i][j] -allocation[i][j];
        }
    }
    return need;
}

// Corrected safety check
bool isSafeState(const vector<vector<int>> &allocation, const vector<vector<int>> &need, const vector<int> &available,vector<int> &safeSequence)
 {
    int nProcesses= allocation.size();
    int nResources =available.size();


    vector<int> work= available;
    vector<bool> finish(nProcesses,false);
    safeSequence.clear();

    int finishedCount= 0;

    while(finishedCount< nProcesses)
     {
        bool found =false;

        for(int i = 0; i < nProcesses; ++i)
         {
            if(!finish[i]) 
            {
        bool canFinish = true;
        for(int j = 0;j < nResources; ++j)
                 {
         if(need[i][j] > work[j]) 
                    {
            canFinish = false;
        break;
                }
                }

     if(canFinish)
      {
        for(int j = 0; j < nResources; ++j)
         {
         work[j] += allocation[i][j];}
        finish[i] = true;
        safeSequence.push_back(i);
        finishedCount++;
         found = true;
                }
            }
        }

        if(!found) 
        {
            return false;
        }
    }

    return true;
}

bool requestResources(int attempt, vector<int> &request, vector<vector<int>> &allocation, vector<vector<int>> &need, vector<int> &available,vector<int> &safeSequenceAfter)
 {
    int nResources =available.size();

    //Checking the  request<= need
    for(int j =0; j< nResources; ++j) 
    {
    if(request[j]> need[attempt][j])
         {
     cout << "Error: Process " << attempt << " has requested more than its maximum need." << endl;
            return false;
        }
    }

    // Check request <= available
    for(int j = 0; j < nResources;++j)
     {
    if(request[j] > available[j])
         {
     cout << "Request cannot be granted immediately ,not enough available resources." << endl;
    return false;
        }
    }

    // Saveing the  old state
    vector<int> oldAvailable = available;
    vector<vector<int>> oldAllocation = allocation;
    vector<vector<int>> oldNeed = need;

    //Pretending  to allocate
    for(int j =0; j < nResources;  ++j)
     {
    available[j] -=request[j];
    allocation[attempt][j] += request[j];
    need[attempt][j] -= request[j];
    }

    // Checking  if  it is safe
    vector<int> safeSeqTest;
    bool safe =isSafeState(allocation, need, available, safeSeqTest);

    if(!safe)
     {
 cout <<"Request would lead to an unsafe state. Rolling back." << endl;

available= oldAvailable;
allocation = oldAllocation;
 need= oldNeed;
 return false;
    }

safeSequenceAfter= safeSeqTest;
return true;
}

int main(int argc, char *argv[])
 {
    if (argc != 2) {
        cerr << "Usage: "<< argv[0] << " <input_file.txt>" << endl;
        return 1;
    }

    int nProcesses, nResources;
    vector<vector<int>>allocation, maxMatrix;
    vector<int> available;

    readInput(argv[1], nProcesses, nResources, allocation, maxMatrix, available);

    vector<vector<int>> need =computeNeed(maxMatrix, allocation);

    vector<int> safeSequence;
    bool safe =isSafeState(allocation, need, available, safeSequence);

    if (safe)
     {
    cout << "System is in a SAFE state." << endl;
    cout << "Safe sequence: ";
    for(size_t i = 0; i < safeSequence.size(); ++i) 
    {
            cout << "P" << safeSequence[i];
            if (i + 1 < safeSequence.size()) cout << " -> ";
        }
        cout << endl;
    } else
     {
        cout << "System is in an UNSAFE state. No safe sequence exists." << endl;
    }

    char choice;
    cout << "Do you want to simulate a resource request? (y/n): ";
    if (cin >> choice && (choice == 'y'||choice =='Y')) 
    {
        int attempt;
        cout << "Enter process ID (0 to " << (nProcesses - 1) << "):";
        cin >> attempt;

        if(attempt <0 ||attempt >= nProcesses) 
        {
            cout << "Invalid process ID." << endl;
            return 0;
        }

     vector<int> request(nResources);
    cout << "Enter request vector of size " << nResources << ": ";
    for (int j = 0; j < nResources; ++j) {
        cin >> request[j];
        }

     vector<int> safeSeqAfter;
     if (requestResources(attempt, request, allocation, need, available, safeSeqAfter)) {
    cout <<"Request can be granted. New safe sequence: ";
            for (size_t i = 0; i < safeSeqAfter.size(); ++i) {
            cout << "P" << safeSeqAfter[i];
            if (i + 1 < safeSeqAfter.size()) cout << " -> ";
            }
            cout << endl;
        } else {
        cout << "Request cannot be granted safely." << endl;
        }
    }

    return 0;
}
