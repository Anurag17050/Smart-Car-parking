#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include<stdbool.h>

#define PARKING_SPACES 50
#define MAX_VEHICLES 100
#define NUM_LEN 15
#define NAME_LEN 50
#define TIME_LEN 20
#define LEN 10
#define BTREE_ORDER 5
#define MAX_KEYS 3 //for sorting 


int static total_vehicles=0;//to keep track of number of vehicles present in the data base

typedef struct parking_space
{
    int parking_space_id;
    int status;// 0 for unoccupied and 1 for occupied
    int occupancy_count;//how many times the particular parking space is used
    int revenue_generated;
} ParkingKey;


typedef struct BTreeNode_parking // B-tree node structure
{
    ParkingKey *keys;         // Array of keys
    int t;                    // Minimum degree (defines the range for number of keys)
    struct BTreeNode_parking **C;     // Array of child pointers
    int n;                    // Current number of keys
    int leaf;                 // Is true when node is leaf. Otherwise false
} BTreeNode_parking;


typedef struct BTree // B-tree structure
{
    struct BTreeNode_parking *root;   // Pointer to root node
    int t;                    // Minimum degree
} BTree;

int minimum_degree()//Function to convert the given B tree order to min degree
{
    int order=BTREE_ORDER; 
    int min_degree;

    min_degree = (int)ceil((double)order / 2);

    return min_degree;
}



BTreeNode_parking* create_node_parking(int t, int leaf) // Create a new node for parking spaces with specified minimum degree 
{
    BTreeNode_parking* new_node = NULL;
    
    new_node = (BTreeNode_parking*)malloc(sizeof(BTreeNode_parking));
    
    if (new_node != NULL) 
    {
        // Initialize B-tree properties
        new_node->t = t;         // Minimum degree
        new_node->leaf = leaf;   // Is this a leaf node
        new_node->n = 0;         // Number of keys currently in the node
        
        // Allocate memory for keys (maximum 2t-1 keys)
        new_node->keys = (ParkingKey*)malloc(sizeof(ParkingKey) * (2*t-1));
        
        if (new_node->keys != NULL) 
        {
            // Allocate memory for child pointers (maximum 2t children)
            new_node->C = (BTreeNode_parking**)malloc(sizeof(BTreeNode_parking*) * (2*t));
            
            if (new_node->C != NULL) 
            {
                // Initialize all child pointers to NULL
                for (int i = 0; i < 2*t; i++) 
                {
                    new_node->C[i] = NULL;
                }
                
                //printf("Created new parking node (t=%d, leaf=%d)\n", t, leaf);
            } 
            else 
            {
                printf("Memory allocation failed for children array!\n");
                free(new_node->keys);
                free(new_node);
                new_node = NULL;
            }
        } 
        else 
        {
            printf("Memory allocation failed for keys array!\n");
            free(new_node);
            new_node = NULL;
        }
    } 
    else 
    {
        printf("Memory allocation failed for new parking node!\n");
    }
    
    return new_node;
}

BTree* createBTree(int t) // Function to create a new B-tree
{
    BTree* tree = (BTree*)malloc(sizeof(BTree));
    if (tree == NULL) 
    {
        printf("Memory allocation failed!\n");
    }
    else
    {
    tree->t = t;
    tree->root = NULL;
    }
    return tree;
}

BTree* parking_tree=NULL;

BTreeNode_parking* search_parking(BTreeNode_parking* root, int parking_space_id) // Function to search a key in the B-tree
{
    int i = 0;
    
    
    while (i < root->n && parking_space_id > root->keys[i].parking_space_id)// Find the first key greater than or equal to parking_space_id
    {    
        i++;
    }
    
    if (i < root->n && parking_space_id == root->keys[i].parking_space_id)// If the found key equals parking_space_id, return this node
    {
            return root;
    }
    
    if (root->leaf)// If key is not found and this is a leaf node
        return NULL;
    
    
    return search_parking(root->C[i], parking_space_id);// Go to the appropriate child
}

void split_child_parking(BTreeNode_parking* parent, int index, BTreeNode_parking* child)// Split child function updated to use the node's minimum degree
{
    // Get the minimum degree from the node
    int t = child->t;
    
    // Create a new node which is going to store t-1 keys of child
    BTreeNode_parking* new_node = create_node_parking(t, child->leaf);
    new_node->n = t - 1;
    
    // Copy the last t-1 keys of child to new_node with all parking data fields
    for (int j = 0; j < t - 1; j++) 
    {
        new_node->keys[j].parking_space_id = child->keys[j + t].parking_space_id;
        new_node->keys[j].status = child->keys[j + t].status;
        new_node->keys[j].occupancy_count = child->keys[j + t].occupancy_count;
        new_node->keys[j].revenue_generated = child->keys[j + t].revenue_generated;
    }
    
    
    if (!child->leaf) // Copy the last t children of child to new_node if it's not a leaf
    {
        for (int j = 0; j < t; j++)
        {
            new_node->C[j] = child->C[j + t];
        }
    }
    
   
    child->n = t - 1; // Reduce the number of keys in child
    
    
    for (int j = parent->n; j >= index + 1; j--)// Create space for the new child in parent
    {
        parent->C[j + 1] = parent->C[j];
    }
    
    parent->C[index + 1] = new_node;// Link the new child to parent
    
    // Move a key from child to parent - find location and move all greater keys one space ahead
    for (int j = parent->n - 1; j >= index; j--) 
    {
        parent->keys[j + 1].parking_space_id = parent->keys[j].parking_space_id;
        parent->keys[j + 1].status = parent->keys[j].status;
        parent->keys[j + 1].occupancy_count = parent->keys[j].occupancy_count;
        parent->keys[j + 1].revenue_generated = parent->keys[j].revenue_generated;
    }
    
    // Copy the middle key of child to parent with all parking data fields
    parent->keys[index].parking_space_id = child->keys[t - 1].parking_space_id;
    parent->keys[index].status = child->keys[t - 1].status;
    parent->keys[index].occupancy_count = child->keys[t - 1].occupancy_count;
    parent->keys[index].revenue_generated = child->keys[t - 1].revenue_generated;
    
    // Increment count of keys in parent
    parent->n++;
}

void insert_nonfull_parking(BTreeNode_parking* node, ParkingKey key) // Insert a key into a non-full node for parking spaces
{
    // Initialize index as the rightmost element
    int i = node->n - 1;
    
    // If this is a leaf node
    if (node->leaf) 
    {
        // Find the location of new key to be inserted
        // Move all greater keys to one place ahead
        while (i >= 0 && node->keys[i].parking_space_id > key.parking_space_id) 
        {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        
        // Insert the new key at found location with all data fields
        node->keys[i + 1].parking_space_id = key.parking_space_id;
        node->keys[i + 1].status = key.status;
        node->keys[i + 1].occupancy_count = key.occupancy_count;
        node->keys[i + 1].revenue_generated = key.revenue_generated;
        node->n++;
        //printf("Inserted parking space ID %d into leaf node\n", key.parking_space_id);
    } 
    else 
    {
        // Find the child which is going to have the new key
        while (i >= 0 && node->keys[i].parking_space_id > key.parking_space_id)
            i--;
            
        i++;
        
        // See if the found child is full
        if (node->C[i]->n == 2 * node->t - 1) {
            // If the child is full, split it
            split_child_parking(node, i, node->C[i]);
            
            // After split, the middle key goes up and the node splits into two.
            // See which of the two is going to have the new key
            if (node->keys[i].parking_space_id < key.parking_space_id)
                i++;
        }
        
        insert_nonfull_parking(node->C[i], key);
    }
}

void insert_parking(ParkingKey key) // Update the insert functions to use the new create_node functions
{
    // If tree is empty
    if (parking_tree->root == NULL) 
    {
        // Create a new root node with degree t
        
        BTreeNode_parking* new_root = create_node_parking(parking_tree->t, 1);
        
        // Explicitly assign each field
        new_root->keys[0].parking_space_id = key.parking_space_id;
        new_root->keys[0].status = key.status;
        new_root->keys[0].occupancy_count = key.occupancy_count;
        new_root->keys[0].revenue_generated = key.revenue_generated;
        
        new_root->n = 1;
        parking_tree->root = new_root;
        //printf("Created new root with parking space ID %d\n", key.parking_space_id);
        return;
    }
    
    // If root is full, tree grows in height
    if (parking_tree->root->n == 2 * parking_tree->t - 1) 
    {
        // Create new root with degree t
        BTreeNode_parking* new_root = create_node_parking(parking_tree->t, 0);
        new_root->C[0] = parking_tree->root;
        
        // Split the old root and move one key to the new root
        split_child_parking(new_root, 0, parking_tree->root);
        
        // New root has two children now. Decide which child is going to have the new key
        int i = 0;
        if (new_root->keys[0].parking_space_id < key.parking_space_id)
            i++;
            
        insert_nonfull_parking(new_root->C[i], key);
        
        parking_tree->root = new_root;
    } 
    else 
    {
        // If root is not full, call insert_nonfull for root
        insert_nonfull_parking(parking_tree->root, key);
    }
}

void displayTree_parking(BTreeNode_parking* node, int level) // Function to display the B-tree (inorder traversal)
{
    if (node == NULL) return;
    
    int i;
    for (i = 0; i < node->n; i++) 
    {
        // Print left child first
        if (!node->leaf)
            displayTree_parking(node->C[i], level + 1);
        
        // Print node information with indentation
        for (int j = 0; j < level; j++) printf("    ");
        printf("ID: %d, Status: %d, Occupancy: %d, Revenue: $%d\n", 
               node->keys[i].parking_space_id, node->keys[i].status, 
               node->keys[i].occupancy_count, node->keys[i].revenue_generated);
    }
    
    // Print rightmost child
    if (!node->leaf)
        displayTree_parking(node->C[i], level + 1);
}

void loadParkingData() 
{
    FILE *file = fopen("parking.txt", "r");
    if (!file) 
    {
        printf("Error opening parking.txt file!\n");
        return;
    }
    
    int parking_space_id, status, occupancy_count, revenue_generated;
    
    
    // Read and process each line from the file
    while (fscanf(file, "%d %d %d %d", &parking_space_id, &status, &occupancy_count, &revenue_generated) == 4) {
    
       // printf("Processing entry #%d:ID=%d Status=%d Occupancy=%d Revenue=%d\n", 
              // count, parking_space_id, status, occupancy_count, revenue_generated);
        
        // Create a key for the parking space
        ParkingKey key;
        key.parking_space_id = parking_space_id;
        key.status = status;
        key.occupancy_count = occupancy_count;
        key.revenue_generated = revenue_generated;
        
        // Insert into B-tree
        insert_parking(key);
    }
    
    //printf("Total entries processed: %d\n", count);
    
    fclose(file);
}

typedef struct vehicle_info
{
    char vehicle_num[NUM_LEN];
    char owner_name[NUM_LEN];
    char arrival_time[TIME_LEN];//entering the time in the format YYYY-MM-DD HH:minmin
    char exit_time[TIME_LEN];
    char membership[LEN];
    int total_parking_hours;
    int parking_space_id;
    float total_amount_paid;
    int parking_count;//number of times the vehicle is being parked
    int vehicle_id;
}VehicleKey;

typedef struct BTreeNode_vehicle // B-tree node structure for Vehicle
{
    VehicleKey *keys;         // Array of keys
    int t;                    // Minimum degree (defines the range for number of keys)
    struct BTreeNode_vehicle **C;     // Array of child pointers
    int n;                    // Current number of keys
    int leaf;                 // Is true when node is leaf. Otherwise false
} BTreeNode_vehicle;

typedef struct BTree_vehicle // B-tree structure for Vehicle
{
    struct BTreeNode_vehicle *root;   // Pointer to root node, update to match BTree
    int t;                    // Minimum degree
} BTree_vehicle;

BTreeNode_vehicle* create_node_vehicle(int t, int leaf) // Create a new node for vehicles with specified minimum degree
{
    BTreeNode_vehicle* new_node = NULL;
    
    new_node = (BTreeNode_vehicle*)malloc(sizeof(BTreeNode_vehicle));
    
    if (new_node != NULL) 
    {
        // Initialize B-tree properties
        new_node->t = t;         // Minimum degree
        new_node->leaf = leaf;   // Is this a leaf node
        new_node->n = 0;         // Number of keys currently in the node
        
        // Allocate memory for keys (maximum 2t-1 keys)
        new_node->keys = (VehicleKey*)malloc(sizeof(VehicleKey) * (2*t-1));
        
        if (new_node->keys != NULL) 
        {
            // Allocate memory for child pointers (maximum 2t children)
            new_node->C = (BTreeNode_vehicle**)malloc(sizeof(BTreeNode_vehicle*) * (2*t));
            
            if (new_node->C != NULL) {
                // Initialize all child pointers to NULL
                for (int i = 0; i < 2*t; i++) 
                {
                    new_node->C[i] = NULL;
                }
                
               // printf("Created new vehicle node (t=%d, leaf=%d)\n", t, leaf);
            } 
            else 
            {
                printf("Memory allocation failed for children array!\n");
                free(new_node->keys);
                free(new_node);
                new_node = NULL;
            }
        } 
        else 
        {
            printf("Memory allocation failed for keys array!\n");
            free(new_node);
            new_node = NULL;
        }
    } 
    else 
    {
        printf("Memory allocation failed for new vehicle node!\n");
    }
    
    return new_node;
}

BTree_vehicle* createBTree_vehicle(int t) // Function to create a new B-tree for vehicle
{
    BTree_vehicle* tree = (BTree_vehicle*)malloc(sizeof(BTree_vehicle));
    if (tree == NULL) 
    {
        printf("Memory allocation failed!\n");
        return NULL;
    }
    
    tree->t = t;
    tree->root = NULL;
    
    return tree;
}

BTree_vehicle* vehicle_tree=NULL;

BTreeNode_vehicle* search_vehicle_by_id(BTreeNode_vehicle* root, int vehicle_id) // Function to search a vehicle in the B-tree by vehicle_id
{
    int i = 0;
    
    // Find the first key greater than or equal to vehicle_id
    while (i < root->n && vehicle_id > root->keys[i].vehicle_id)
    {    
        i++;
    }
    
    // If the found key equals vehicle_id, return this node
    if (i < root->n && vehicle_id == root->keys[i].vehicle_id)
    {
        return root;
    }
    
    // If key is not found and this is a leaf node
    if (root->leaf)
        return NULL;
    
    // Go to the appropriate child
    return search_vehicle_by_id(root->C[i], vehicle_id);
}

void split_child_vehicle(BTreeNode_vehicle* parent, int index, BTreeNode_vehicle* child) // Split child function updated to use the node's minimum degree
{
    // Get the minimum degree from the node
    int t = child->t;
    
    // Create a new node which is going to store t-1 keys of child
    BTreeNode_vehicle* new_node = create_node_vehicle(t, child->leaf);
    new_node->n = t - 1;
    
    // Copy the last t-1 keys of child to new_node with all vehicle data fields
    for (int j = 0; j < t - 1; j++) 
    {
        // Deep copy of all vehicle fields
        new_node->keys[j].vehicle_id = child->keys[j + t].vehicle_id;
        strcpy(new_node->keys[j].vehicle_num, child->keys[j + t].vehicle_num);
        strcpy(new_node->keys[j].owner_name, child->keys[j + t].owner_name);
        strcpy(new_node->keys[j].arrival_time, child->keys[j + t].arrival_time);
        strcpy(new_node->keys[j].exit_time, child->keys[j + t].exit_time);
        strcpy(new_node->keys[j].membership, child->keys[j + t].membership);
        new_node->keys[j].total_parking_hours = child->keys[j + t].total_parking_hours;
        new_node->keys[j].parking_space_id = child->keys[j + t].parking_space_id;
        new_node->keys[j].total_amount_paid = child->keys[j + t].total_amount_paid;
        new_node->keys[j].parking_count = child->keys[j + t].parking_count;
    }
    
    // Copy the last t children of child to new_node if it's not a leaf
    if (!child->leaf) 
    {
        for (int j = 0; j < t; j++)
            new_node->C[j] = child->C[j + t];
    }
    
    // Reduce the number of keys in child
    child->n = t - 1;
    
    // Create space for the new child in parent
    for (int j = parent->n; j >= index + 1; j--)
        parent->C[j + 1] = parent->C[j];
    
    // Link the new child to parent
    parent->C[index + 1] = new_node;
    
    // Move a key from child to parent - find location and move all greater keys one space ahead
    for (int j = parent->n - 1; j >= index; j--) {
        // Deep copy all fields when moving keys
        parent->keys[j + 1].vehicle_id = parent->keys[j].vehicle_id;
        strcpy(parent->keys[j + 1].vehicle_num, parent->keys[j].vehicle_num);
        strcpy(parent->keys[j + 1].owner_name, parent->keys[j].owner_name);
        strcpy(parent->keys[j + 1].arrival_time, parent->keys[j].arrival_time);
        strcpy(parent->keys[j + 1].exit_time, parent->keys[j].exit_time);
        strcpy(parent->keys[j + 1].membership, parent->keys[j].membership);
        parent->keys[j + 1].total_parking_hours = parent->keys[j].total_parking_hours;
        parent->keys[j + 1].parking_space_id = parent->keys[j].parking_space_id;
        parent->keys[j + 1].total_amount_paid = parent->keys[j].total_amount_paid;
        parent->keys[j + 1].parking_count = parent->keys[j].parking_count;
    }
    
    // Copy the middle key of child to parent with all data fields
    parent->keys[index].vehicle_id = child->keys[t - 1].vehicle_id;
    strcpy(parent->keys[index].vehicle_num, child->keys[t - 1].vehicle_num);
    strcpy(parent->keys[index].owner_name, child->keys[t - 1].owner_name);
    strcpy(parent->keys[index].arrival_time, child->keys[t - 1].arrival_time);
    strcpy(parent->keys[index].exit_time, child->keys[t - 1].exit_time);
    strcpy(parent->keys[index].membership, child->keys[t - 1].membership);
    parent->keys[index].total_parking_hours = child->keys[t - 1].total_parking_hours;
    parent->keys[index].parking_space_id = child->keys[t - 1].parking_space_id;
    parent->keys[index].total_amount_paid = child->keys[t - 1].total_amount_paid;
    parent->keys[index].parking_count = child->keys[t - 1].parking_count;
    
    // Increment count of keys in parent
    parent->n++;
}

void insert_nonfull_vehicle(BTreeNode_vehicle* node, VehicleKey key) {
    // Initialize index as the rightmost element
    int i = node->n - 1;
    
    // If this is a leaf node
    if (node->leaf) {
        // Find the location of new key to be inserted
        // Move all greater keys to one place ahead
        while (i >= 0 && node->keys[i].vehicle_id > key.vehicle_id) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        
        // Insert the new key at found location with all vehicle data fields
        node->keys[i + 1].vehicle_id = key.vehicle_id;
        strcpy(node->keys[i + 1].vehicle_num, key.vehicle_num);
        strcpy(node->keys[i + 1].owner_name, key.owner_name);
        strcpy(node->keys[i + 1].arrival_time, key.arrival_time);
        strcpy(node->keys[i + 1].exit_time, key.exit_time);
        strcpy(node->keys[i + 1].membership, key.membership);
        node->keys[i + 1].total_parking_hours = key.total_parking_hours;
        node->keys[i + 1].parking_space_id = key.parking_space_id;
        node->keys[i + 1].total_amount_paid = key.total_amount_paid;
        node->keys[i + 1].parking_count = key.parking_count;
        node->n++;
        
        //printf("Inserted vehicle ID %d, Number %s, Owner %s into leaf node\n", 
               //key.vehicle_id, key.vehicle_num, key.owner_name);
    } else {
        // Find the child which is going to have the new key
        while (i >= 0 && node->keys[i].vehicle_id > key.vehicle_id)
            i--;
            
        i++;
        
        // See if the found child is full
        if (node->C[i]->n == 2 * node->t - 1) {
            // If the child is full, split it
            split_child_vehicle(node, i, node->C[i]);
            
            // After split, the middle key goes up and the node splits into two.
            // See which of the two is going to have the new key
            if (node->keys[i].vehicle_id < key.vehicle_id)
                i++;
        }
        
        insert_nonfull_vehicle(node->C[i], key);
    }
}

void insert_vehicle(VehicleKey key)// Update the insert functions to use the new create_node functions
 {
    // If tree is empty
    
    if (vehicle_tree->root == NULL) 
    {
        // Create a new root node with degree t
        BTreeNode_vehicle* new_root = create_node_vehicle(vehicle_tree->t, 1);
        
        // Explicitly assign each field
        new_root->keys[0].vehicle_id = key.vehicle_id;
        strcpy(new_root->keys[0].vehicle_num, key.vehicle_num);
        strcpy(new_root->keys[0].owner_name, key.owner_name);
        strcpy(new_root->keys[0].arrival_time, key.arrival_time);
        strcpy(new_root->keys[0].exit_time, key.exit_time);
        strcpy(new_root->keys[0].membership, key.membership);
        new_root->keys[0].total_parking_hours = key.total_parking_hours;
        new_root->keys[0].parking_space_id = key.parking_space_id;
        new_root->keys[0].total_amount_paid = key.total_amount_paid;
        new_root->keys[0].parking_count = key.parking_count;
        
        new_root->n = 1;
        vehicle_tree->root = new_root;
        //printf("Created new root with vehicle ID %d\n", key.vehicle_id);
        return;
    }
    // If root is full, tree grows in height
    if (vehicle_tree->root->n == 2 * vehicle_tree->t - 1) 
    {
        // Create new root with degree t
        BTreeNode_vehicle* new_root = create_node_vehicle(vehicle_tree->t, 0);
        new_root->C[0] = vehicle_tree->root;
        
        // Split the old root and move one key to the new root
        split_child_vehicle(new_root, 0, vehicle_tree->root);
        
        // New root has two children now. Decide which child is going to have the new key
        int i = 0;
        if (new_root->keys[0].vehicle_id < key.vehicle_id)
            i++;
            
        insert_nonfull_vehicle(new_root->C[i], key);
        
        vehicle_tree->root = new_root;
    } 
    else 
    {
        // If root is not full, call insert_nonfull for root
        insert_nonfull_vehicle(vehicle_tree->root, key);
    }
}

void displayTree_vehicle(BTreeNode_vehicle* node, int level) // Function to display the B-tree for vehicle (inorder traversal)
{
    if (node == NULL) return;
    
    int i;
    for (i = 0; i < node->n; i++) 
    {
        // Print left child first
        if (!node->leaf)
            displayTree_vehicle(node->C[i], level + 1);
        
        // Print node information with indentation
        for (int j = 0; j < level; j++) printf("    ");
        printf("Vehicle ID: %d, Number: %s, Owner: %s, Parking Space: %d\n", 
               node->keys[i].vehicle_id, node->keys[i].vehicle_num, 
               node->keys[i].owner_name, node->keys[i].parking_space_id);
    }
    
    // Print rightmost child
    if (!node->leaf)
        displayTree_vehicle(node->C[i], level + 1);
}

void loadVehicleData() // Function to load vehicle data from file
{
    FILE *file = fopen("vehicle.txt", "r");
    if (!file) 
    {
        printf("Error opening vehicles.txt file!\n");
        return;
    }
    int vehicle_id,parking_space_id,parking_count,total_parking_hours;
    float total_amount_paid;
    char vehicle_num[NUM_LEN];
    char owner_name[NUM_LEN];
    char arrival_time[TIME_LEN],exit_time[TIME_LEN];
    char membership[LEN];
    VehicleKey key;
    
    // Read and process each line from the file
    while (fscanf(file, "%d %s %s %s %s %s %d %d %f %d", 
                 &vehicle_id, vehicle_num, owner_name, 
                 arrival_time, exit_time, membership,
                 &total_parking_hours,&parking_space_id, &total_amount_paid, &parking_count) == 10) 
    {
        //printf("Processing vehicle #%d: ID=%d, Number=%s, Owner=%s\n", 
               //count, key.vehicle_id, key.vehicle_num, key.owner_name);
        
        // Insert into B-tree
        key.vehicle_id=vehicle_id;
        strcpy(key.vehicle_num,vehicle_num);
        strcpy(key.owner_name,owner_name);
        strcpy(key.arrival_time,arrival_time);
        strcpy(key.exit_time,exit_time);
        strcpy(key.membership,membership);
        key.total_parking_hours=total_parking_hours;
        key.parking_space_id=parking_space_id;
        key.total_amount_paid=total_amount_paid;
        key.parking_count=parking_count;
        insert_vehicle(key);
        total_vehicles++;
    }
    
    //printf("Total vehicles processed: %d\n", count);
    
    fclose(file);
}

int searchParkingSpace(BTreeNode_parking* node, int space_id, BTreeNode_parking** result_node, int* position) 
{
    if (node == NULL) 
    {
        return 0;
    }
    
    int i = 0;
    
    // Find the first key greater than or equal to space_id
    while (i < node->n && space_id > node->keys[i].parking_space_id) 
    {
        i++;
    }
    
    // Check if the key was found
    if (i < node->n && space_id == node->keys[i].parking_space_id) 
    {
        *result_node = node;
        *position = i;
        return 1;
    }
    
    // If this is a leaf node and key not found, return 0
    if (node->leaf) 
    {
        return 0;
    }
    
    // Recursively search the appropriate child
    return searchParkingSpace(node->C[i], space_id, result_node, position);
}

void writeParkingDataToFile(BTreeNode_parking* node, FILE* file) 
{
    if (node == NULL) 
    {
        return;
    }
    
    int i;
    // First traverse to leftmost child if not a leaf
    if (!node->leaf) {
        writeParkingDataToFile(node->C[0], file);
    }
    
    // Process all keys in this node and write them to file
    for (i = 0; i < node->n; i++) {
        // Write all parking attributes in the same format as used for reading
        fprintf(file, "%d %d %d %d\n", 
                node->keys[i].parking_space_id,
                node->keys[i].status,
                node->keys[i].occupancy_count,
                node->keys[i].revenue_generated);
        
        // If not a leaf, process the child to the right of current key
        if (!node->leaf) {
            writeParkingDataToFile(node->C[i+1], file);
        }
    }
}

void save_parking_data_btree() 
{
    FILE *file = fopen("parking.txt", "w");
    if (!file) 
    {
        printf("Error opening parking.txt\n");
        return;
    }
    
    // Start recursion from root to write all parking data
    if (parking_tree->root != NULL) 
    {
        writeParkingDataToFile(parking_tree->root, file);
    }
    
    fclose(file);
    //printf("Parking data saved successfully.\n");
}

void update_parking_space_btree(int id, int new_status, int additional_revenue) 
{
    // First, search for the parking space in the B-tree
    BTreeNode_parking* result_node = NULL;
    int position = -1;
    int found = searchParkingSpace(parking_tree->root, id, &result_node, &position);
    
    if (!found || result_node == NULL) 
    {
        printf("Parking space with ID %d not found in the B-tree\n", id);
        return;
    }
    
    // Create temporary file
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Error opening temp file\n");
        return;
    }
    
    // Update the parking space data in the B-tree
    result_node->keys[position].status = new_status;
    result_node->keys[position].occupancy_count++;
    result_node->keys[position].revenue_generated += additional_revenue;
    
    // Write the entire updated B-tree to the temporary file
    if (parking_tree->root != NULL) {
        writeParkingDataToFile(parking_tree->root, temp);
    }
    
    fclose(temp);
    
    // Replace the original file with the updated one
    remove("parking.txt");
    rename("temp.txt", "parking.txt");
}

void update_parking_space_exit_btree(int id, int new_status, int additional_revenue, int parking) {
    // First, search for the parking space in the B-tree
    BTreeNode_parking* result_node = NULL;
    int position = -1;
    int found = searchParkingSpace(parking_tree->root, id, &result_node, &position);
    
    if (!found || result_node == NULL) {
        printf("Parking space with ID %d not found in the B-tree\n", id);
        return;
    }
    
    // Create temporary file
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Error opening temp file\n");
        return;
    }
    
    // Update the parking space data in the B-tree
    result_node->keys[position].status = new_status;
    result_node->keys[position].occupancy_count = parking; // Set directly to the parking value
    result_node->keys[position].revenue_generated += additional_revenue;
    
    // Write the entire updated B-tree to the temporary file
    if (parking_tree->root != NULL) 
    {
        writeParkingDataToFile(parking_tree->root, temp);
    }
    
    fclose(temp);
    
    // Replace the original file with the updated one
    remove("parking.txt");
    rename("temp.txt", "parking.txt");
}

void searchVehicleByNumber(BTreeNode_vehicle* node, const char* vehicle_num, BTreeNode_vehicle** result_node, int* position)
{
    if (node == NULL) 
    {
        return;
    }
    
    int i;
    // Check all keys in the current node
    for (i = 0; i < node->n; i++) {
        if (strcmp(node->keys[i].vehicle_num, vehicle_num) == 0) {
            *result_node = node;
            *position = i;
            return;
        }
    }
    
    // If node is a leaf, we're done searching
    if (node->leaf) {
        return;
    }
    
    // Search all children recursively
    for (i = 0; i <= node->n; i++) {
        searchVehicleByNumber(node->C[i], vehicle_num, result_node, position);
        // If found in a child, stop searching
        if (*result_node != NULL) {
            return;
        }
    }
}

void writeVehicleDataToFile(BTreeNode_vehicle* node, FILE* file) 
{
    if (node == NULL || file == NULL) 
    {
        return;
    }
    
    // First traverse to leftmost child if not a leaf
    if (!node->leaf) 
    {
        writeVehicleDataToFile(node->C[0], file);
    }
    
    // Process all keys in this node and write them to file
    for (int i = 0; i < node->n; i++) {
        // Validate string fields before writing
        char vehicle_num[NUM_LEN] ;
        char owner_name[NUM_LEN] ;
        char arrival_time[TIME_LEN] ;
        char exit_time[TIME_LEN] ;
        char membership[LEN] ;
        
        // Copy strings safely with validation
        //if (node->keys[i].vehicle_num != NULL && strlen(node->keys[i].vehicle_num) > 0) {
            strcpy(vehicle_num, node->keys[i].vehicle_num);
           // vehicle_num[NUM_LEN - 1] = '\0';  // Ensure null termination
        //}
        
       // if (node->keys[i].owner_name != NULL && strlen(node->keys[i].owner_name) > 0) {
            strcpy(owner_name, node->keys[i].owner_name);
           // owner_name[NUM_LEN - 1] = '\0';
        //}
        
        //if (node->keys[i].arrival_time != NULL && strlen(node->keys[i].arrival_time) > 0) 
        //{
            strcpy(arrival_time, node->keys[i].arrival_time);
           // arrival_time[TIME_LEN - 1] = '\0';
        //}
        
        //if (node->keys[i].exit_time != NULL && strlen(node->keys[i].exit_time) > 0) {
            strcpy(exit_time, node->keys[i].exit_time);
         //   exit_time[TIME_LEN - 1] = '\0';
        //}
        
        //if (node->keys[i].membership != NULL && strlen(node->keys[i].membership) > 0) 
        //{
            strcpy(membership, node->keys[i].membership);
            //membership[LEN - 1] = '\0';
        //}
        
        // Write data with validated strings
        fprintf(file, "%d %s %s %s %s %s %d %d %.2f %d\n",
                node->keys[i].vehicle_id,
                vehicle_num,
                owner_name,
                arrival_time,
                exit_time,
                membership,
                node->keys[i].total_parking_hours,
                node->keys[i].parking_space_id,
                node->keys[i].total_amount_paid,
                node->keys[i].parking_count);
        
        // If not a leaf, process the child to the right of current key
        if (!node->leaf) {
            writeVehicleDataToFile(node->C[i+1], file);
        }
    }
}

void save_vehicle_data_btree() 
{
    FILE* file = fopen("vehicle.txt", "w");  // Open file for writing
    if (file == NULL) {
        printf("Failed to open file for writing\n");
        return;
    }
    
    // Start recursion from root to write all vehicle data
    if (vehicle_tree != NULL && vehicle_tree->root != NULL) {
        writeVehicleDataToFile(vehicle_tree->root, file);
        printf("Vehicle data saved successfully.\n");
    } else {
        printf("No vehicle data to save (empty tree).\n");
    }
    
    fclose(file);  // Close the file after writing
}

void update_vehicle_btree(char num[], char arr_time[], int parking_hours, char exit_time[], float amount_paid,  char membership_status[],int parking_id) {
    // First find the vehicle in the B-tree
    // Since the tree is likely indexed by vehicle_id (int), we need a way to find by vehicle_num
    BTreeNode_vehicle* current = vehicle_tree->root;
    BTreeNode_vehicle* found_node = NULL;
    int found_idx = -1;
    
    // Open temp file for writing
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Error opening temp file\n");
        return;
    }
    
    int found = 0;

    // Search function to find vehicle by number in B-tree
    searchVehicleByNumber(vehicle_tree->root, num, &found_node, &found_idx);
    
    if (found_node != NULL && found_idx != -1) 
    {
        found = 1;
        // Update vehicle details
        strncpy(found_node->keys[found_idx].arrival_time, arr_time, 
                sizeof(found_node->keys[found_idx].arrival_time) - 1);
        found_node->keys[found_idx].arrival_time[sizeof(found_node->keys[found_idx].arrival_time) - 1] = '\0';
        
        strncpy(found_node->keys[found_idx].exit_time, exit_time, 
                sizeof(found_node->keys[found_idx].exit_time) - 1);
        found_node->keys[found_idx].exit_time[sizeof(found_node->keys[found_idx].exit_time) - 1] = '\0';
        
        found_node->keys[found_idx].total_parking_hours += parking_hours;
        found_node->keys[found_idx].total_amount_paid += amount_paid;
        
        strncpy(found_node->keys[found_idx].membership, membership_status, 
                sizeof(found_node->keys[found_idx].membership) - 1);
        found_node->keys[found_idx].membership[sizeof(found_node->keys[found_idx].membership) - 1] = '\0';
        
        found_node->keys[found_idx].parking_count++;
        found_node->keys[found_idx].parking_space_id=parking_id;
    }
    
    // Write updated B-tree to temp file
    if (vehicle_tree->root != NULL) {
        writeVehicleDataToFile(vehicle_tree->root, temp);
    }
    
    fclose(temp);
    
    if (!found) {
        printf("Vehicle with number %s not found\n", num);
        remove("temp.txt");
        return;
    }
    
    // Replace original file
    if (remove("vehicle.txt") != 0) {
        printf("Error deleting old file\n");
        return;
    }
    if (rename("temp.txt", "vehicle.txt") != 0) {
        printf("Error renaming temp file\n");
    }
}

void update_vehicle_exit_btree(char num[], char arr_time[], int parking_hours, char exit_time[], float amount_paid, char membership_status[], int parking) {
    // Find the vehicle in the B-tree
    BTreeNode_vehicle* found_node = NULL;
    int found_idx = -1;
    
    // Open temp file for writing
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        printf("Error opening temp file\n");
        return;
    }
    
    int found = 0;

    // Search function to find vehicle by number in B-tree
    searchVehicleByNumber(vehicle_tree->root, num, &found_node, &found_idx);
    
    if (found_node != NULL && found_idx != -1) 
    {
        found = 1;
        // Update vehicle details
        strncpy(found_node->keys[found_idx].arrival_time, arr_time, 
                sizeof(found_node->keys[found_idx].arrival_time) - 1);
        found_node->keys[found_idx].arrival_time[sizeof(found_node->keys[found_idx].arrival_time) - 1] = '\0';
        
        strncpy(found_node->keys[found_idx].exit_time, exit_time, 
                sizeof(found_node->keys[found_idx].exit_time) - 1);
        found_node->keys[found_idx].exit_time[sizeof(found_node->keys[found_idx].exit_time) - 1] = '\0';
        
        found_node->keys[found_idx].total_parking_hours = parking_hours;
        found_node->keys[found_idx].total_amount_paid += amount_paid;
        
        strncpy(found_node->keys[found_idx].membership, membership_status, 
                sizeof(found_node->keys[found_idx].membership) - 1);
        found_node->keys[found_idx].membership[sizeof(found_node->keys[found_idx].membership) - 1] = '\0';
        
        // Set parking count directly to provided value
        found_node->keys[found_idx].parking_count = parking;
    }
    
    // Write updated B-tree to temp file
    if (vehicle_tree->root != NULL) {
        writeVehicleDataToFile(vehicle_tree->root, temp);
    }
    
    fclose(temp);
    
    if (!found) {
        printf("Vehicle with number %s not found\n", num);
        remove("temp.txt");
        return;
    }
    
    // Replace original file
    if (remove("vehicle.txt") != 0) {
        printf("Error deleting old file\n");
        return;
    }
    if (rename("temp.txt", "vehicle.txt") != 0) {
        printf("Error renaming temp file\n");
    }
}

void Vehicle_entry(char* vehicle_num, int vehicle_id, char* owner_name, char* entry_time)
{
    BTreeNode_vehicle* vehicle_node= NULL;
    int found_key=0;

    if (vehicle_tree->root!=NULL)
    {
        vehicle_node = search_vehicle_by_id(vehicle_tree->root, vehicle_id);
    }
    
    if(vehicle_node==NULL)//create a new node and insert in it
    {
        VehicleKey key;
        total_vehicles++;
        key.vehicle_id=total_vehicles;
        strcpy(key.vehicle_num,vehicle_num);
        strcpy(key.owner_name,owner_name);
        strcpy(key.arrival_time,entry_time);
        strcpy(key.membership,"NULL");
        strcpy(key.exit_time,"IN_PARKING");
        key.total_amount_paid=0;
        key.total_parking_hours=0;

        //search for the parking space
        int available_parking_space=-1;

        if(parking_tree->root!=NULL)
        {
            BTreeNode_parking* current = parking_tree->root;
            int found=0;
            for(int i=21;i<=50 && found==0 ;i++)//for no membership
            {
                BTreeNode_parking* Node=search_parking(current,i);
                if(Node!=NULL)//if we found a Node that is space that is present
                {
                    for(int p=0;p<Node->n;p++)
                    {
                        if(Node->keys[p].status==0)//present and unoccupied
                        {
                            found=1;
                            available_parking_space=i;
                            key.parking_space_id=i;
                            key.parking_count=1;
                            //Update the parking space details
                           // Node->keys[p].occupancy_count++;//as another vehicle inserted count will increase
                            Node->keys[p].status=1;//occupied
                            update_parking_space_btree(i,1,0);
                        }
                    }
                }

                else//create a Node for that parking space
                {
                    found=1;
                    available_parking_space=i;
                    key.parking_space_id=i;
                    key.parking_count=1;
                    ParkingKey park_key;
                    park_key.occupancy_count=1;
                    park_key.parking_space_id=i;
                    park_key.revenue_generated=0;
                    park_key.status=1;
                    insert_parking(park_key);
                    save_parking_data_btree();
                }
            }
        }
        if(available_parking_space!=-1)
        {
            printf("Vehicle parked successfully at %d and vehicle_id is : %d\n",available_parking_space,total_vehicles);
            insert_vehicle(key);
            save_vehicle_data_btree();
           // printf("insertion done\n");
            //displayTree_vehicle(vehicle_tree->root,0);
            //printf("display done");
            
        }
        else
        {
            printf("Vehicle is not parked , no parking space");
        }
        
    }

    else// Vehicle is already present in our data base
    {
        int flag=0;
        int j;
        for( j=0; j<vehicle_node->n && flag==0 ;j++)
        {
            if(strcmp(vehicle_node->keys[j].vehicle_num,vehicle_num)==0)
            {
                flag=1;
            }
        }

        if(strcmp(vehicle_node->keys[j-1].membership,"GOLDEN")==0)//Golden membership holder
        {
            //searching for a parking space
            if(parking_tree->root!=NULL)
            {
                BTreeNode_parking* current = parking_tree->root;
                int found1=0;
                int k;
                for(k=1;k<=10 && found1==0 ;k++)//for GOLDEN membership
                {
                    BTreeNode_parking* Node= search_parking(current,k);
                    if(Node!=NULL)//if we found a Node that is space that is present
                    {
                        for(int i=0;i<Node->n;i++)
                        {
                            if(Node->keys[i].status==0)//present and unoccupied
                            {
                                found1=1;
                                vehicle_node->keys[j].parking_space_id=k;
                                update_vehicle_btree(vehicle_num,entry_time,0,"IN_PARKING",0,"GOLDEN",k);
                                //vehicle_node->keys[j].parking_count++;
                                //Update the parking space details
                               /// Node->keys[i].occupancy_count++;//as another vehicle inserted count will increase
                                Node->keys[i].status=1;//occupied
                                update_parking_space_btree(k,1,0);
                                printf("Vehicle parked successfully at %d",k);
                            }
                        }
                    }
    
                    else//create a Node for that parking space
                    {
                        found1=1;
                        vehicle_node->keys[j].parking_space_id=k;
                        //vehicle_node->keys[j].parking_count++;
                        update_vehicle_btree(vehicle_num,entry_time,0,"IN_PARKING",0,"GOLDEN",k);
                        ParkingKey park_key;
                        park_key.occupancy_count=1;
                        park_key.parking_space_id=k;
                        park_key.revenue_generated=0;
                        park_key.status=1;
                        insert_parking(park_key);
                        save_parking_data_btree();
                        printf("Vehicle parked successfully at %d",k);
                    }
                    
                }
                
            }
            
        }

        else if(strcmp(vehicle_node->keys[j-1].membership,"SILVER")==0)//silver membership holder
        {
            //searching for a parking space
            if(parking_tree->root!=NULL)
            {
                BTreeNode_parking* current = parking_tree->root;
                int found1=0;
                int k;
                for(k=11;k<=20 && found1==0 ;k++)//for SILVER membership
                {
                    BTreeNode_parking* Node=search_parking(current,k);
                    if(Node!=NULL)//if we found a Node that is space that is present
                    {
                        for(int i=0;i<Node->n;i++)
                        {
                            if(Node->keys[i].status==0)//present and unoccupied
                            {
                                found1=1;
                                vehicle_node->keys[j].parking_space_id=k;
                                //vehicle_node->keys[j].parking_count++;
                                update_vehicle_btree(vehicle_num,entry_time,0,"IN_PARKING",0,"SILVER",k);
                                //Update the parking space details
                                //Node->keys[i].occupancy_count++;//as another vehicle inserted count will increase
                                Node->keys[i].status=1;//occupied
                                update_parking_space_btree(k,1,0);
                                printf("Vehicle parked successfully at %d",k);
                            }
                        }
                    }
    
                    else//create a Node for that parking space
                    {
                        found1=1;
                        vehicle_node->keys[j].parking_space_id=k;
                       // vehicle_node->keys[j].parking_count++;
                       update_vehicle_btree(vehicle_num,entry_time,0,"IN_PARKING",0,"SILVER",k);
                        ParkingKey park_key;
                        park_key.occupancy_count=1;
                        park_key.parking_space_id=k;
                        park_key.revenue_generated=0;
                        park_key.status=1;
                        insert_parking(park_key);
                        save_parking_data_btree();
                        printf("Vehicle parked successfully at %d",k);
                    }
                    //printf("Vehicle parked successfully at %d",k);
                }
                
            }
            
        }

        else 
        {
            //searching for a parking space
            if(parking_tree->root!=NULL)
            {
                BTreeNode_parking* current = parking_tree->root;
                int found1=0;
                int k;
                for(k=21;k<=50 && found1==0 ;k++)//for NULL membership
                {
                    BTreeNode_parking* Node=search_parking(current,k);
                    if(Node!=NULL)//if we found a Node that is space that is present
                    {
                        for(int i=0;i<Node->n;i++)
                        {
                            if(Node->keys[i].status==0)//present and unoccupied
                            {
                                found1=1;
                                vehicle_node->keys[j].parking_space_id=k;
                                //vehicle_node->keys[j].parking_count++;
                                update_vehicle_btree(vehicle_num,entry_time,0,"IN_PARKING",0,"NULL",k);
                                //Update the parking space details
                                //Node->keys[i].occupancy_count++;//as another vehicle inserted count will increase
                                Node->keys[i].status=1;//occupied
                                update_parking_space_btree(k,1,0);
                                printf("Vehicle parked successfully at %d",k);
                            }
                        }
                    }
    
                    else//create a Node for that parking space
                    {
                        found1=1;
                        vehicle_node->keys[j].parking_space_id=k;
                        //vehicle_node->keys[j].parking_count++;
                        update_vehicle_btree(vehicle_num,entry_time,0,"IN_PARKING",0,"NULL",k);
                        ParkingKey park_key;
                        park_key.occupancy_count=1;
                        park_key.parking_space_id=k;
                        park_key.revenue_generated=0;
                        park_key.status=1;
                        insert_parking(park_key);
                        save_parking_data_btree();
                        printf("Vehicle parked successfully at %d",k);
                    }
                    //printf("Vehicle parked successfully at %d",k);
                }
                
            }
        }
    }
}

void parse_date_time(char *dateTime, int *day, int *month, int *year, int *hour, int *minute)
{
    sscanf(dateTime, "%d-%d-%d_%d:%d", day, month, year, hour, minute);
   // parsing is commonly used to convert input data (like strings) into a more useful or manageable format (like variables or data structures). 
   //For example, when parsing a date-time string, the program extracts the individual components (day, month, year, hour, minute) from the string so they can be used in calculations or manipulations.
   //sscanf is used in C for parsing formatted input from a string. 
   //It reads data from a string and stores the values in specified variables based on a format we provide
   
}

double calculate_hours(int day1, int month1, int year1, int hour1, int minute1, 
    int day2, int month2, int year2, int hour2, int minute2) 
{
struct tm arrival_time = {0};
struct tm exit_time = {0};

//struct tm 
//{
//    int tm_sec;   // seconds after the minute (0-60)
//    int tm_min;   // minutes after the hour (0-59)
//    int tm_hour;  // hours since midnight (0-23)
//    int tm_mday;  // day of the month (1-31)
//    int tm_mon;   // months since January (0-11)
//    int tm_year;  // years since 1900
//    int tm_wday;  // days since Sunday (0-6)
//    int tm_yday;  // days since January 1 (0-365)
//    int tm_isdst; // Daylight Saving Time flag
//};(this is the structure which is already defined in time.h header)

arrival_time.tm_mday = day1;
arrival_time.tm_mon = month1 - 1;  
arrival_time.tm_year = year1 - 1900;
arrival_time.tm_hour = hour1;
arrival_time.tm_min = minute1;

exit_time.tm_mday = day2;
exit_time.tm_mon = month2 - 1;
exit_time.tm_year = year2 - 1900;
exit_time.tm_hour = hour2;
exit_time.tm_min = minute2;

// Convert to time_t
    //time_t is a data type used to represent time as a number of seconds since the "epoch" (00:00:00 UTC on January 1, 1970). 
    //It is typically implemented as a long integer. This format allows for easy calculations and comparisons of time.
    //mktime is a function that converts tm structure to time_t value

time_t t1 = mktime(&arrival_time);
time_t t2 = mktime(&exit_time);

if (t1 == -1 || t2 == -1) 
{
fprintf(stderr, "Error: Unable to convert time\n");
return -1; 
}

// Calculate the difference in hours as a floating-point value
double difference_in_seconds = difftime(t2, t1);
double difference_in_hours = difference_in_seconds / 3600.0;

return difference_in_hours;
}

void Vehicle_exit(int parking_space_id, int vehicle_id,char vehicle_num[],char owner_name[],char departure_time[])
{
    int arrival_day, arrival_month, arrival_year, arrival_hour, arrival_minute;
    int departure_day, departure_month, departure_year, departure_hour, departure_minute;

    BTreeNode_vehicle* node=search_vehicle_by_id(vehicle_tree->root,vehicle_id);
    if(node==NULL)
    {
        printf("Vehicle is not present in the parking , check the details again\n");
    }

    else
    {
        int found=0;
        int i;
        for(i=0;i<node->n && found==0;i++)
        {
            if((vehicle_id==node->keys[i].vehicle_id) && strcmp(vehicle_num,node->keys[i].vehicle_num)==0)
            {
                found=1;
            }

            else{
                printf("not found\n");
            }
        }
        //int parking_id=node->keys[i-1].parking_space_id;
        BTreeNode_parking* parking_node=search_parking(parking_tree->root,parking_space_id);
        int j;
        int flag=0;
        for(j=0;j<parking_node->n && flag==0;j++)
        {
            if(parking_space_id==parking_node->keys[j].parking_space_id)
            {
                flag=1;
            }
            else{
                printf("parking space not found\n");
            }
        }
        parse_date_time(node->keys[i-1].arrival_time,&arrival_day,&arrival_month,&arrival_year,&arrival_hour,&arrival_minute);
        parse_date_time(departure_time,&departure_day,&departure_month,&departure_year,&departure_hour,&departure_minute);

        int hours_spent =calculate_hours(arrival_day, arrival_month, arrival_year, arrival_hour, arrival_minute, departure_day, departure_month, departure_year, departure_hour, departure_minute);
        printf("Number of hours spent: %d",hours_spent);
        node->keys[i-1].total_parking_hours+= hours_spent;

        //update the membership if applicable
        if(node->keys[i-1].total_parking_hours > 200)
        {
            if(strcmp(node->keys[i-1].membership,"GOLDEN")!=0)
            {
                strcpy(node->keys[i-1].membership,"GOLDEN");
                printf("NOW YOU ARE A GOLDEN MEMBERSHIP HOLDER\n");
            }
        }

        else if(node->keys[i-1].total_parking_hours>100)
        {
            if(strcmp(node->keys[i-1].membership,"SILVER")!=0)
            {
                strcpy(node->keys[i-1].membership,"SILVER");
                printf("NOW YOU ARE A SILVER MEMBERSHIP HOLDER\n");
            }
        }
        
        // Caluculating parking charges
        double charges = 0.0;
        if(strcmp(node->keys[i-1].membership, "NULL")==0)
        {
            if(hours_spent <=3)
            {
                charges = 100.0;
            }
            else
            {
                charges=(hours_spent - 3) * 50.0 + 100.0;
            }
    
            printf("Parking charges : %.2lf Rs\n", charges);
        }
    
        else// for both golden and premium membership holders
        {
            if(hours_spent <= 3)
            {
                charges = 90.0;
            }
            else
            {
                charges = 0.9* ((hours_spent-3) * 50.0 + 100.0);//10% discount
            }
            printf("Parking Charges: %.2lf Rs\n" , charges);
        }

        //node->keys[i-1].total_amount_paid+=charges;

        update_vehicle_exit_btree(vehicle_num,node->keys[i-1].arrival_time,node->keys[i-1].total_parking_hours,departure_time,charges,node->keys[i-1].membership,node->keys[i-1].parking_count);
        parking_node->keys[j-1].status=0;
        update_parking_space_exit_btree(parking_space_id,parking_node->keys[j-1].status,charges,parking_node->keys[j-1].occupancy_count);
    }
}



void freeBTreeNode_parking(BTreeNode_parking* node) // Function to free a single B-tree node
{
    if (node == NULL) return;
    
    // Free all keys in this node
    if (node->keys != NULL) {
        free(node->keys);
    }
    
    // If not a leaf, free all children first
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++) {
            freeBTreeNode_parking(node->C[i]);
        }
    }
    
    // Free the children array
    if (node->C != NULL) {
        free(node->C);
    }
    
    // Finally free the node itself
    free(node);
}


void freeBTree_parking(BTree* tree) // Function to free the entire B-tree
{
    if (tree == NULL) return;
    
    // Free all nodes starting from root
    freeBTreeNode_parking(tree->root);
    
    // Free the tree structure itself
    free(tree);
}


void freeBTreeNode_vehicle(BTreeNode_vehicle* node) // Function to free a single B-tree node for vehicle info
{
    if (node == NULL) return;
    
    // Free all keys in this node
    if (node->keys != NULL) {
        free(node->keys);
    }
    
    // If not a leaf, recursively free all children first
    if (!node->leaf) {
        for (int i = 0; i <= node->n; i++) {
            freeBTreeNode_vehicle(node->C[i]);
        }
    }
    
    // Free the children array
    if (node->C != NULL) {
        free(node->C);
    }
    
    // Finally free the node itself
    free(node);
}


void freeBTree_vehicle(BTree_vehicle* tree) // Function to free the entire vehicle B-tree
{
    if (tree == NULL) return;
    
    // Free all nodes starting from root
    freeBTreeNode_vehicle(tree->root);
    
    // Free the tree structure itself
    free(tree);
}

// For sorting vehicle_data

typedef struct VehicleRecord {
    int vehicle_id;
    char vehicle_num[20];
    char owner_name[50];
    char arrival_time[50];
    char exit_time[50];
    char membership[20];
    int hours_spent;
    int parking_id;
    float amount_paid;
    int parking_count;
} VehicleRecord;

typedef struct BTree_Node_vehicle {
    void *keys;              // Array of keys (float for amount_paid, int for parking_count)
    VehicleRecord **records;  // Array of records
    struct BTree_Node_vehicle **children; // Array of child pointers
    int num_keys;           // Current number of keys
    bool is_leaf;           // Is true when node is leaf
    bool is_amount_tree;    // True if this is the amount_paid tree
} BTree_Node_vehicle;

// Function to create a new B-tree node
BTree_Node_vehicle* create_Node_vehicle(bool is_leaf, bool is_amount_tree) {
    BTree_Node_vehicle* newNode = (BTree_Node_vehicle*)malloc(sizeof(BTree_Node_vehicle));
    if (is_amount_tree) {
        newNode->keys = malloc(sizeof(float) * MAX_KEYS);
    } else {
        newNode->keys = malloc(sizeof(int) * MAX_KEYS);
    }
    newNode->records = (VehicleRecord**)malloc(sizeof(VehicleRecord*) * MAX_KEYS);
    newNode->children = (BTree_Node_vehicle**)malloc(sizeof(BTree_Node_vehicle*) * (MAX_KEYS + 1));
    newNode->num_keys = 0;
    newNode->is_leaf = is_leaf;
    newNode->is_amount_tree = is_amount_tree;
    return newNode;
}

// Function to print a vehicle record
void printRecord_vehicle(VehicleRecord* record) {
    printf("ID: %d | Num: %s | Owner: %s | Amount: %.2f | Count: %d\n",
           record->vehicle_id, record->vehicle_num, record->owner_name,
           record->amount_paid,
           record->parking_count);
}

// Function to traverse the B-tree (in-order) and print records
void traverseAndPrint_vehicle(BTree_Node_vehicle* node) 
{
    if (node == NULL) return;
    
    int i;
    for (i = 0; i < node->num_keys; i++) {
        if (!node->is_leaf) {
            traverseAndPrint_vehicle(node->children[i]);
        }
        
        if (node->is_amount_tree) {
            printf("Amount Paid: %.2f -> ", ((float*)node->keys)[i]);
        } else {
            printf("Parking Count: %d -> ", ((int*)node->keys)[i]);
        }
        printRecord_vehicle(node->records[i]);
    }
    
    if (!node->is_leaf) {
        traverseAndPrint_vehicle(node->children[i]);
    }
}


// Function to traverse the B-tree and print records within an amount range
void traverseAndPrintAmountRange_vehicle(BTree_Node_vehicle* node, float min_amount, float max_amount) {
    if (node == NULL) return;
    
    int i;
    for (i = 0; i < node->num_keys; i++) {
        if (!node->is_leaf) {
            traverseAndPrintAmountRange_vehicle(node->children[i], min_amount, max_amount);
        }
        
        // Check if this is the amount tree (important!)
        if (node->is_amount_tree) {
            float current_amount = ((float*)node->keys)[i];
            
            // Only proceed if we're within the range
            if (current_amount >= min_amount && current_amount <= max_amount) {
                printf("Amount Paid: %.2f -> ", current_amount);
                printRecord_vehicle(node->records[i]);
            }
            
            // If we've passed the max amount, we can stop early
            if (current_amount > max_amount) {
                return;
            }
        }
    }
    
    if (!node->is_leaf) {
        traverseAndPrintAmountRange_vehicle(node->children[i], min_amount, max_amount);
    }
}

// Function to split a full child of a node
void splitChild_vhicles(BTree_Node_vehicle* parent, int index, BTree_Node_vehicle* child) {
    BTree_Node_vehicle* newNode = create_Node_vehicle(child->is_leaf, child->is_amount_tree);
    newNode->num_keys = MAX_KEYS / 2;
    
    // Copy the right half of child's keys and records to newNode
    for (int i = 0; i < newNode->num_keys; i++) {
        if (child->is_amount_tree) {
            ((float*)newNode->keys)[i] = ((float*)child->keys)[i + MAX_KEYS / 2 + 1];
        } else {
            ((int*)newNode->keys)[i] = ((int*)child->keys)[i + MAX_KEYS / 2 + 1];
        }
        newNode->records[i] = child->records[i + MAX_KEYS / 2 + 1];
    }
    
    // Copy the right half of child's children to newNode if not leaf
    if (!child->is_leaf) {
        for (int i = 0; i < newNode->num_keys + 1; i++) {
            newNode->children[i] = child->children[i + MAX_KEYS / 2 + 1];
        }
    }
    
    child->num_keys = MAX_KEYS / 2;
    
    // Make space for the new child in parent
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = newNode;
    
    // Move parent's keys and records to make space
    for (int i = parent->num_keys - 1; i >= index; i--) {
        if (parent->is_amount_tree) {
            ((float*)parent->keys)[i + 1] = ((float*)parent->keys)[i];
        } else {
            ((int*)parent->keys)[i + 1] = ((int*)parent->keys)[i];
        }
        parent->records[i + 1] = parent->records[i];
    }
    
    // Copy the middle key and record of child to parent
    if (child->is_amount_tree) {
        ((float*)parent->keys)[index] = ((float*)child->keys)[MAX_KEYS / 2];
    } else {
        ((int*)parent->keys)[index] = ((int*)child->keys)[MAX_KEYS / 2];
    }
    parent->records[index] = child->records[MAX_KEYS / 2];
    parent->num_keys++;
}

// Function to insert a key into a non-full node
void insertNonFull_vehicles(BTree_Node_vehicle* node, void* key, VehicleRecord* record) {
    int i = node->num_keys - 1;
    
    if (node->is_leaf) {
        // Find the location to insert the new key
        if (node->is_amount_tree) {
            float key_val = *(float*)key;
            while (i >= 0 && key_val < ((float*)node->keys)[i]) {
                ((float*)node->keys)[i + 1] = ((float*)node->keys)[i];
                node->records[i + 1] = node->records[i];
                i--;
            }
            ((float*)node->keys)[i + 1] = key_val;
        } else {
            int key_val = *(int*)key;
            while (i >= 0 && key_val < ((int*)node->keys)[i]) {
                ((int*)node->keys)[i + 1] = ((int*)node->keys)[i];
                node->records[i + 1] = node->records[i];
                i--;
            }
            ((int*)node->keys)[i + 1] = key_val;
        }
        node->records[i + 1] = record;
        node->num_keys++;
    } else {
        // Find the child which is going to have the new key
        if (node->is_amount_tree) {
            float key_val = *(float*)key;
            while (i >= 0 && key_val < ((float*)node->keys)[i]) {
                i--;
            }
        } else {
            int key_val = *(int*)key;
            while (i >= 0 && key_val < ((int*)node->keys)[i]) {
                i--;
            }
        }
        i++;
        
        // Check if the found child is full
        if (node->children[i]->num_keys == MAX_KEYS) {
            splitChild_vhicles(node, i, node->children[i]);
            
            // After split, decide which child to go to
            if (node->is_amount_tree) {
                if (*(float*)key > ((float*)node->keys)[i]) {
                    i++;
                }
            } else {
                if (*(int*)key > ((int*)node->keys)[i]) {
                    i++;
                }
            }
        }
        
        insertNonFull_vehicles(node->children[i], key, record);
    }
}

// Function to insert a key into the B-tree
void insert_vehicles(BTree_Node_vehicle** root, void* key, VehicleRecord* record, bool is_amount_tree) {
    BTree_Node_vehicle* node = *root;
    
    if (node == NULL) {
        *root = create_Node_vehicle(true, is_amount_tree);
        if (is_amount_tree) {
            ((float*)(*root)->keys)[0] = *(float*)key;
        } else {
            ((int*)(*root)->keys)[0] = *(int*)key;
        }
        (*root)->records[0] = record;
        (*root)->num_keys = 1;
        return;
    }
    
    if (node->num_keys == MAX_KEYS) {
        BTree_Node_vehicle* newNode = create_Node_vehicle(false, is_amount_tree);
        newNode->children[0] = node;
        *root = newNode;
        splitChild_vhicles(newNode, 0, node);
        
        // Decide which child to insert into
        int i = 0;
        if (is_amount_tree) {
            if (*(float*)key > ((float*)newNode->keys)[0]) {
                i++;
            }
        } else {
            if (*(int*)key > ((int*)newNode->keys)[0]) {
                i++;
            }
        }
        insertNonFull_vehicles(newNode->children[i], key, record);
    } else {
        insertNonFull_vehicles(node, key, record);
    }
}

// Function to parse date-time string (simplified for this example)
void parseDateTime(const char* input, char* output) {
    // Just copy for now - implement proper parsing if needed
    strncpy(output, input, 49);
    output[49] = '\0';
}

// Function to load data from vehicle.txt and build both trees
void loadVehicleData_sorting(BTree_Node_vehicle** amount_tree, BTree_Node_vehicle** count_tree) {
    FILE* file = fopen("vehicle.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    VehicleRecord* record;
    char line[256];
    
    while (fgets(line, sizeof(line), file)) {
        record = (VehicleRecord*)malloc(sizeof(VehicleRecord));
        
        // Parse the line (simplified parsing - adjust as needed for your exact format)
        int parsed = sscanf(line, "%d %19s %49[^0-9] %49s %49s %19s %d %d %f %d",
                           &record->vehicle_id,
                           record->vehicle_num,
                           record->owner_name,
                           record->arrival_time,
                           record->exit_time,
                           record->membership,
                           &record->hours_spent,
                           &record->parking_id,
                           &record->amount_paid,
                           &record->parking_count);
        
        if (parsed == 10) {
            // Clean up owner name (remove leading/trailing whitespace)
            char *p = record->owner_name;
            while (*p == ' ' || *p == '\t') p++;
            memmove(record->owner_name, p, strlen(p) + 1);
            
            p = record->owner_name + strlen(record->owner_name) - 1;
            while (p >= record->owner_name && (*p == ' ' || *p == '\t' || *p == '\n')) {
                *p-- = '\0';
            }
            
            // Insert into amount_paid tree
            float amount_key = record->amount_paid;
            insert_vehicles(amount_tree, &amount_key, record, true);
            
            // Insert into parking_count tree
            int count_key = record->parking_count;
            insert_vehicles(count_tree, &count_key, record, false);
        } else {
            free(record);
        }
    }
    
    fclose(file);
}

// Function to free memory allocated for the B-tree
void freeTree(BTree_Node_vehicle* node) {
    if (node == NULL) return;
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            freeTree(node->children[i]);
        }
    }
    
    free(node->keys);
    free(node->records);
    free(node->children);
    free(node);
}

// Function to free all vehicle records (call after both trees are freed)
void freeAllRecords(BTree_Node_vehicle* node) {
    if (node == NULL) return;
    
    for (int i = 0; i < node->num_keys; i++) {
        free(node->records[i]);
    }
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            freeAllRecords(node->children[i]);
        }
    }
}

//for sorting parkind data

typedef struct ParkingRecord {
    int parking_id;
    int occupancy_status;
    int parking_count;
    int revenue;
} ParkingRecord;

typedef struct BTree_Node_parking {
    int *keys;              // Array of keys
    ParkingRecord **records; // Array of records (for parking_count or revenue)
    struct BTree_Node_parking **children; // Array of child pointers
    int num_keys;           // Current number of keys
    bool is_leaf;           // Is true when node is leaf
} BTree_Node_parking;

// Function to create a new B-tree node
BTree_Node_parking* createNode_sort_parking(bool is_leaf) {
    BTree_Node_parking* newNode = (BTree_Node_parking*)malloc(sizeof(BTree_Node_parking));
    newNode->keys = (int*)malloc(sizeof(int) * MAX_KEYS);
    newNode->records = (ParkingRecord**)malloc(sizeof(ParkingRecord*) * MAX_KEYS);
    newNode->children = (BTree_Node_parking**)malloc(sizeof(BTree_Node_parking*) * (MAX_KEYS + 1));
    newNode->num_keys = 0;
    newNode->is_leaf = is_leaf;
    return newNode;
}

// Function to traverse the B-tree (in-order) and print records
void traverseAndPrint_sort_parking(BTree_Node_parking* node, const char* key_type) {
    if (node == NULL) return;
    
    int i;
    for (i = 0; i < node->num_keys; i++) {
        if (!node->is_leaf) {
            traverseAndPrint_sort_parking(node->children[i], key_type);
        }
        printf("Key (%s): %d | Parking ID: %d, Occupancy: %d, Count: %d, Revenue: %d\n",
               key_type, node->keys[i],
               node->records[i]->parking_id,
               node->records[i]->occupancy_status,
               node->records[i]->parking_count,
               node->records[i]->revenue);
    }
    
    if (!node->is_leaf) {
        traverseAndPrint_sort_parking(node->children[i], key_type);
    }
}

// Function to split a full child of a node
void splitChild_sort_parking(BTree_Node_parking* parent, int index, BTree_Node_parking* child) {
    BTree_Node_parking* newNode = createNode_sort_parking(child->is_leaf);
    newNode->num_keys = MAX_KEYS / 2;
    
    // Copy the right half of child's keys and records to newNode
    for (int i = 0; i < newNode->num_keys; i++) {
        newNode->keys[i] = child->keys[i + MAX_KEYS / 2 + 1];
        newNode->records[i] = child->records[i + MAX_KEYS / 2 + 1];
    }
    
    // Copy the right half of child's children to newNode if not leaf
    if (!child->is_leaf) {
        for (int i = 0; i < newNode->num_keys + 1; i++) {
            newNode->children[i] = child->children[i + MAX_KEYS / 2 + 1];
        }
    }
    
    child->num_keys = MAX_KEYS / 2;
    
    // Make space for the new child in parent
    for (int i = parent->num_keys; i > index; i--) {
        parent->children[i + 1] = parent->children[i];
    }
    parent->children[index + 1] = newNode;
    
    // Move parent's keys and records to make space
    for (int i = parent->num_keys - 1; i >= index; i--) {
        parent->keys[i + 1] = parent->keys[i];
        parent->records[i + 1] = parent->records[i];
    }
    
    // Copy the middle key and record of child to parent
    parent->keys[index] = child->keys[MAX_KEYS / 2];
    parent->records[index] = child->records[MAX_KEYS / 2];
    parent->num_keys++;
}

// Function to insert a key into a non-full node
void insertNonFull_sort_parking(BTree_Node_parking* node, int key, ParkingRecord* record, bool is_count_tree) {
    int i = node->num_keys - 1;
    
    if (node->is_leaf) {
        // Find the location to insert the new key
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            node->records[i + 1] = node->records[i];
            i--;
        }
        
        // For duplicates, we insert them as they come
        node->keys[i + 1] = key;
        node->records[i + 1] = record;
        node->num_keys++;
    } else {
        // Find the child which is going to have the new key
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        
        // Check if the found child is full
        if (node->children[i]->num_keys == MAX_KEYS) {
            splitChild_sort_parking(node, i, node->children[i]);
            
            // After split, decide which child to go to
            if (key > node->keys[i]) {
                i++;
            }
        }
        
        insertNonFull_sort_parking(node->children[i], key, record, is_count_tree);
    }
}

// Function to insert a key into the B-tree
void insert_sort_parking(BTree_Node_parking** root, int key, ParkingRecord* record, bool is_count_tree) {
    BTree_Node_parking* node = *root;
    
    if (node == NULL) {
        *root = createNode_sort_parking(true);
        (*root)->keys[0] = key;
        (*root)->records[0] = record;
        (*root)->num_keys = 1;
        return;
    }
    
    if (node->num_keys == MAX_KEYS) {
        BTree_Node_parking* newNode = createNode_sort_parking(false);
        newNode->children[0] = node;
        *root = newNode;
        splitChild_sort_parking(newNode, 0, node);
        
        // Decide which child to insert into
        int i = 0;
        if (key > newNode->keys[0]) {
            i++;
        }
        insertNonFull_sort_parking(newNode->children[i], key, record, is_count_tree);
    } else {
        insertNonFull_sort_parking(node, key, record, is_count_tree);
    }
}

// Function to load data from parking.txt and build both trees
void loadParkingData_sort_parking(BTree_Node_parking** count_tree, BTree_Node_parking** revenue_tree) {
    FILE* file = fopen("parking.txt", "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    
    ParkingRecord* record;
    while (!feof(file)) {
        record = (ParkingRecord*)malloc(sizeof(ParkingRecord));
        if (fscanf(file, "%d %d %d %d", 
                  &record->parking_id, 
                  &record->occupancy_status, 
                  &record->parking_count, 
                  &record->revenue) == 4) {
            
            // Insert into count tree (key = parking_count)
            insert_sort_parking(count_tree, record->parking_count, record, true);
            
            // Insert into revenue tree (key = revenue)
            insert_sort_parking(revenue_tree, record->revenue, record, false);
        } else {
            free(record);
            break;
        }
    }
    
    fclose(file);
}

// Function to free memory allocated for the B-tree
void freeTree_sort_parking(BTree_Node_parking* node) {
    if (node == NULL) return;
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            freeTree_sort_parking(node->children[i]);
        }
    }
    
    // We don't free the records here because they're shared between trees
    free(node->keys);
    free(node->records);
    free(node->children);
    free(node);
}

// Function to free all parking records (call after both trees are freed)
void freeAllRecords_sort_parking(BTree_Node_parking* node) {
    if (node == NULL) return;
    
    for (int i = 0; i < node->num_keys; i++) {
        free(node->records[i]);
    }
    
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            freeAllRecords_sort_parking(node->children[i]);
        }
    }
}

int main()
{
    int min_degree= minimum_degree();
    parking_tree=createBTree(min_degree);
    vehicle_tree=createBTree_vehicle(min_degree);

    loadParkingData(parking_tree);
    loadVehicleData(vehicle_tree);

    printf("------------------------\n");
    printf("WELCOME TO A1 PARKIN SPACE\n");
    printf("-------------------------\n");

    displayTree_vehicle(vehicle_tree->root,0);
    displayTree_parking(parking_tree->root,0);

    printf("DATA LOADED SUCCESSFULLY\n");
    printf(" \n");
    printf("-------------------------\n");

    char vehicle_num[NUM_LEN];
    char name[NAME_LEN];
    char arrival_time[TIME_LEN];// DD-MM-YYYY HH:MM
    char departure_time[TIME_LEN];
    int vehicle_id;
    int parking_space_id;
    int preference;
    float min_amount;
    float max_amount;
    BTree_Node_vehicle* amount_tree = NULL;
    BTree_Node_vehicle* parking_count_tree = NULL;

    BTree_Node_parking* count_tree = NULL;
    BTree_Node_parking* revenue_tree = NULL;


    do
    {
        printf("\n1.Entry of a Vehicle\n");
        printf("2.Exit of a vehicle\n");
        printf("3.Sort vehicles by parking count and amount_paid with in the range\n");
        printf("4.Sort Parking spaces by occupancy and revenue generated\n");
        printf("5.To exit\n");
        printf("Enter your preference\n");
        scanf("%d",&preference);

        switch(preference)
        {
            case 1:
            printf("Enter the vehicle details\n");
            printf("Vehicle number:\n");
            scanf(" %[^\n]",vehicle_num);
            printf("Enter vehicle_id if it is there ,if not there enter 0\n");
            scanf("%d",&vehicle_id);
            printf("Owner Name:\n");
            scanf(" %[^\n]",name);
            printf("Entry Time\n");
            scanf(" %[^\n]",arrival_time);
            Vehicle_entry(vehicle_num,vehicle_id,name,arrival_time);
            break;

            case 2:
            printf("enter details foe vehicle exit\n");
            printf("Vehicle number:\n");
            scanf(" %[^\n]",vehicle_num);
            printf("Enter vehicle_id \n");
            scanf("%d",&vehicle_id);
            printf("Owner Name:\n");
            scanf(" %[^\n]",name);
            printf("enter parking space id\n");
            scanf("%d",&parking_space_id);
            printf("Departure Time\n");
            scanf(" %[^\n]",departure_time);
           
            Vehicle_exit(parking_space_id,vehicle_id,vehicle_num,name,departure_time);
            break;

            case 3:
            printf("Enter minimum amount\n");
            scanf("%f",&min_amount);
            printf("Enter maximum amount\n");
            scanf("%f",&max_amount);
    
            // Load data from file and build both trees
            loadVehicleData_sorting(&amount_tree, &parking_count_tree);
    
            // Print both trees
            printf("\nAmount Paid Tree (sorted by amount_paid):\n");
            traverseAndPrintAmountRange_vehicle(amount_tree,min_amount,max_amount);
    
            printf("\nParking Count Tree (sorted by parking_count):\n");
            traverseAndPrint_vehicle(parking_count_tree);
    
            // Free memory
            freeTree(amount_tree);
            // Records are shared, so we free them when freeing count_tree
            //freeAllRecords(parking_count_tree->records);
            freeTree(parking_count_tree);
            break;

            case 4:
            
            
            // Load data from file and build both trees
            loadParkingData_sort_parking(&count_tree, &revenue_tree);
            
            // Print both trees
            printf("Parking Count Tree (sorted by parking_count):\n");
            traverseAndPrint_sort_parking(count_tree, "parking_count");
            
            printf("\nRevenue Tree (sorted by revenue):\n");
            traverseAndPrint_sort_parking(revenue_tree, "revenue");
            
            // Free memory
            freeTree_sort_parking(count_tree);
            freeTree_sort_parking(revenue_tree);
            break;

            default:
            printf("invalid\n");
            break;

        }
    } while (preference!=5);
    
        //free the parking_tree and vehicle_tree
        freeBTree_parking(parking_tree);
        freeBTree_vehicle(vehicle_tree);
        printf("memory deallocation done\n");

    return 0;
}