// #ifndef DATABLOCK_H
// #define DATABLOCK_H

// class node {
//     private:
//         // Each node include starting point, number of nunits
//         // process_id, and pointer to next
//         // If the node is free, process_id is 0
//         int start;
//         int num_units;
//         int process_id;
//         node *next;
//         node(int start, int process_id, int num_units, node *pn);

//         friend class MemoryManager;
// };

// class MemoryManager {
//     private:
//         // First and last pointer of memory
//         node *first;

//         // Given memory size is 256KB and each unit is 2KB
//         // 128 units are in memory
//         const int MEMORY_SIZE = 128;

//         // Total count of fragement
//         int total_fragment;

//         // Number of allocation success
//         int allocated_num;

//         // Number of allocation failure
//         int allocate_fail;

//         // Total of nodes that pointer traversed
//         int total_traverses;

//         // Number of deallocation success
//         int deallocate_num;

//         //
//         MemoryManager *firstfit;
//         MemoryManager *bestfit;
//         MemoryManager *worstfit;
//         MemoryManager *nextfit;

//     public:
//         // Constructor
//         MemoryManager();

//         // Destructor
//         ~MemoryManager();

//         // Allocation for each method
//         bool allocateFF(int process_id, int num_units);

//         bool allocateBF(int process_id, int num_units);

//         bool allocateWF(int process_id, int num_units);

//         bool allocateNF(int process_id, int num_units);

//         // Deallocate memory that has given process id
//         bool deallocate_mem (const MemoryManager* method, int process_id);

//         // Count fragment number
//         int fragment_count(const MemoryManager* method);

//         // Display statistic
//         void print(const MemoryManager* method);

//         // Test
//         void test(const MemoryManager* method, int process_id, int num_units);

//         char getMethodName(const MemoryManager* method);
// };

// #endif