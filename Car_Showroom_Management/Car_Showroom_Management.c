#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#define MAX_SHOWROOMS 3
#define MAX_STRING 100
#define B_PLUS_ORDER 5  // Order of B+ tree

// Enums for car types
typedef enum {
    HATCHBACK,
    SEDAN,
    SUV
} CarType;

typedef enum {
    PETROL,
    DIESEL,
    CNG,
    ELECTRIC,
    HYBRID
} FuelType;

typedef enum {
    CASH,
    LOAN
} PaymentType;

// Car structure
typedef struct Car {
    char VIN[20];              // Vehicle Identification Number (primary key)
    char name[MAX_STRING];     // Name of car
    char color[MAX_STRING];    // Color of car
    double price;              // Price in lakhs
    FuelType fuelType;         // Fuel type
    CarType carType;           // Type of car (Hatchback, Sedan, SUV)
    bool isSold;               // Whether car is sold or not
    int showroomId;            // Showroom ID where car belongs
} Car;

// Customer structure
typedef struct Customer {
    char name[MAX_STRING];           // Name of customer
    char mobileNo[15];               // Mobile number
    char address[MAX_STRING];        // Address
    char VIN[20];                    // VIN of sold car
    char registrationNo[20];         // Car registration number
    PaymentType paymentType;         // Payment type (Cash/Loan)
    int emiMonths;                   // EMI duration in months (if applicable)
    double downPayment;              // Down payment (if loan)
    double loanAmount;               // Loan amount (if loan)
    double emiAmount;                // Monthly EMI amount (if loan)
} Customer;

// Sales Person structure
typedef struct SalesPerson {
    int id;                           // Sales person ID
    char name[MAX_STRING];            // Name of sales person
    double salesTarget;               // Monthly sales target (50 lakhs)
    double salesAchieved;             // Sales achieved so far
    double commission;                // Commission earned (2% of sales achieved)
    int numSales;                     // Number of cars sold
    bool extraIncentive;              // Additional 1% incentive for top performer
} SalesPerson;

// Showroom structure
typedef struct Showroom {
    int id;                           // Unique showroom ID
    char name[MAX_STRING];            // Name of showroom
    char manufacturer[MAX_STRING];    // Car manufacturer
    int numTotalCars;                 // Total number of cars
    int numAvailableCars;             // Number of available cars
    int numSoldCars;                  // Number of sold cars
    double totalSales;                // Total sales in rupees
    double lastMonthSales;            // Sales for last month
    double twoMonthsAgoSales;         // Sales for two months ago
    double threeMonthsAgoSales;       // Sales for three months ago
    int lastMonthCars;                // Number of cars sold last month
    int twoMonthsAgoCars;             // Number of cars sold two months ago
    int threeMonthsAgoCars;           // Number of cars sold three months ago
} Showroom;

// Node structure for B+ Tree
typedef struct BPlusTreeNode {
    bool isLeaf;                          // Is it a leaf node
    int numKeys;                          // Number of keys in node
    char keys[B_PLUS_ORDER - 1][20];      // Keys (VINs for cars, mobileNo for customers)
    void* data[B_PLUS_ORDER - 1];         // Data pointers (for leaf nodes)
    struct BPlusTreeNode* children[B_PLUS_ORDER]; // Children pointers
    struct BPlusTreeNode* next;           // Next leaf node (for leaf nodes only)
    int order;
} BPlusTreeNode;

// B+ Tree structure
typedef struct BPlusTree {
    BPlusTreeNode* root;
    int type; // 1 - Car, 2 - Customer, 3 - SalesPerson, 4 - Showroom
    int order;
} BPlusTree;

// Structure to hold per-salesperson customer trees
typedef struct {
    char salesPersonId[50];  // Salesperson ID (e.g., "1_101")
    BPlusTree* customerTree; // B+ tree for this salesperson’s customers
} SalesPersonCustomerTree;

typedef struct {
    Car* car;
    int showroomId;
} ShowroomCar;

// Global data structures
BPlusTree* carTree;           // Tree for all cars
BPlusTree* availableCarTree;  // Tree for available cars
BPlusTree* soldCarTree;       // Tree for sold cars
SalesPersonCustomerTree* salesPersonCustomerTrees = NULL; // Array of customer trees
int numSalesPersonTrees = 0;  // Number of salesperson customer trees
BPlusTree* salesPersonTrees[MAX_SHOWROOMS] = {NULL}; // Array of trees, one per showroom
BPlusTree* showroomTree;      // Tree for showrooms

// Function prototypes
BPlusTree* createBPlusTree(int type);
BPlusTreeNode* createBPlusTreeNode(bool isLeaf);
int findKeyPosition(BPlusTreeNode* node, char* key);
void insertIntoBPlusTree(BPlusTree* tree, char* key, void* data);
void* searchInBPlusTree(BPlusTree* tree, char* key);
void splitChild(BPlusTreeNode* parent, int index, BPlusTreeNode* child);
void insertNonFull(BPlusTreeNode* node, char* key, void* data);
void deleteFromBPlusTree(BPlusTree* tree, char* key);
void printBPlusTree(BPlusTree* tree);
BPlusTree* getCustomerTreeForSalesPerson(char* salesPersonId);
char* carTypeToString(CarType type);
char* fuelTypeToString(FuelType type);
char* paymentTypeToString(PaymentType type);
CarType stringToCarType(const char* str);
FuelType stringToFuelType(const char* str);
PaymentType stringToPaymentType(const char* str);
double calculateEMI(double principal, double rate, int time);
void displayCarDetails(Car* car);
void displayCustomerDetails(Customer* customer);
void displaySalesPersonDetails(SalesPerson* person);
void displayShowroomDetails(Showroom* showroom);
void mergeShowroomTrees(BPlusTree* mergedTree, BPlusTree* showroom1, BPlusTree* showroom2, BPlusTree* showroom3);
void searchSalesPersonByRange(BPlusTree* tree, double minSales, double maxSales);
void saveCarsToFile();
void loadCarsFromFile();
void saveCustomersToFile();
void loadCustomersFromFile();
void saveSalesPersonsToFile();
void loadSalesPersonsFromFile();
void saveShowroomsToFile();
void loadShowroomsFromFile();
void addCar(int showroomId, Car* car);
void addSalesPerson(int showroomId, SalesPerson* person);
void mergeAndSortShowroomsByVIN(const char* outputName);
void displayAllCarsShowroomWise();
void displayAllSalesPersonsShowroomWise();
void displayCustomersForSalesPerson(int showroomId, int salesPersonId);

// B+ Tree operations
BPlusTree* createBPlusTree(int type) {
    BPlusTree* tree = (BPlusTree*)malloc(sizeof(BPlusTree));
    if (tree == NULL) {
        fprintf(stderr, "Memory allocation failed for BPlusTree\n");
        exit(EXIT_FAILURE);
    }
    tree->root = NULL;
    tree->type = type;
    tree->order = B_PLUS_ORDER;
    return tree;
}

BPlusTreeNode* createBPlusTreeNode(bool isLeaf) {
    BPlusTreeNode* node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    if (node == NULL) {
        fprintf(stderr, "Memory allocation failed for BPlusTreeNode\n");
        exit(EXIT_FAILURE);
    }
    node->isLeaf = isLeaf;
    node->numKeys = 0;
    node->next = NULL;
    node->order = B_PLUS_ORDER;
    for (int i = 0; i < B_PLUS_ORDER - 1; i++) {
        strcpy(node->keys[i], "");
        node->data[i] = NULL;
    }
    for (int i = 0; i < B_PLUS_ORDER; i++) {
        node->children[i] = NULL;
    }
    return node;
}

int findKeyPosition(BPlusTreeNode* node, char* key) {
    int i = 0;
    while (i < node->numKeys && strcmp(node->keys[i], key) < 0) {
        i++;
    }
    return i;
}

void splitChild(BPlusTreeNode* parent, int index, BPlusTreeNode* child) {
    BPlusTreeNode* newNode = createBPlusTreeNode(child->isLeaf);
    int mid = (B_PLUS_ORDER - 1) / 2;

    for (int i = 0; i < mid; i++) {
        strcpy(newNode->keys[i], child->keys[i + mid]);
        newNode->data[i] = child->data[i + mid];
    }

    if (!child->isLeaf) {
        for (int i = 0; i <= mid; i++) {
            newNode->children[i] = child->children[i + mid];
        }
    }

    newNode->numKeys = mid;
    child->numKeys = mid;

    if (child->isLeaf) {
        newNode->next = child->next;
        child->next = newNode;
    }

    for (int i = parent->numKeys; i > index; i--) {
        strcpy(parent->keys[i], parent->keys[i - 1]);
        parent->data[i] = parent->data[i - 1];
        parent->children[i + 1] = parent->children[i];
    }

    strcpy(parent->keys[index], child->keys[mid - 1]);
    parent->data[index] = child->data[mid - 1];
    parent->children[index + 1] = newNode;
    parent->numKeys++;
}

void insertNonFull(BPlusTreeNode* node, char* key, void* data) {
    int i = node->numKeys - 1;

    if (node->isLeaf) {
        while (i >= 0 && strcmp(node->keys[i], key) > 0) {
            strcpy(node->keys[i + 1], node->keys[i]);
            node->data[i + 1] = node->data[i];
            i--;
        }
        strcpy(node->keys[i + 1], key);
        node->data[i + 1] = data;
        node->numKeys++;
    } else {
        while (i >= 0 && strcmp(node->keys[i], key) > 0) {
            i--;
        }
        i++;
        if (node->children[i]->numKeys == B_PLUS_ORDER - 1) {
            splitChild(node, i, node->children[i]);
            if (strcmp(key, node->keys[i]) > 0) {
                i++;
            }
        }
        insertNonFull(node->children[i], key, data);
    }
}

void insertIntoBPlusTree(BPlusTree* tree, char* key, void* data) {
    if (tree->root == NULL) {
        tree->root = createBPlusTreeNode(true);
        strcpy(tree->root->keys[0], key);
        tree->root->data[0] = data;
        tree->root->numKeys = 1;
        return;
    }

    if (tree->root->numKeys == B_PLUS_ORDER - 1) {
        BPlusTreeNode* newRoot = createBPlusTreeNode(false);
        newRoot->children[0] = tree->root;
        tree->root = newRoot;
        splitChild(newRoot, 0, newRoot->children[0]);
        int i = 0;
        if (strcmp(newRoot->keys[0], key) < 0) {
            i++;
        }
        insertNonFull(newRoot->children[i], key, data);
    } else {
        insertNonFull(tree->root, key, data);
    }
}

void* searchInBPlusTree(BPlusTree* tree, char* key) {
    if (tree->root == NULL) {
        return NULL;
    }

    BPlusTreeNode* current = tree->root;
    while (!current->isLeaf) {
        int i = 0;
        while (i < current->numKeys && strcmp(key, current->keys[i]) > 0) {
            i++;
        }
        current = current->children[i];
    }

    for (int i = 0; i < current->numKeys; i++) {
        if (strcmp(current->keys[i], key) == 0) {
            return current->data[i];
        }
    }
    return NULL;
}

// Helper function to get or create a customer tree for a salesperson
BPlusTree* getCustomerTreeForSalesPerson(char* salesPersonId) {
    for (int i = 0; i < numSalesPersonTrees; i++) {
        if (strcmp(salesPersonCustomerTrees[i].salesPersonId, salesPersonId) == 0) {
            return salesPersonCustomerTrees[i].customerTree;
        }
    }
    SalesPersonCustomerTree* temp = (SalesPersonCustomerTree*)realloc(salesPersonCustomerTrees, 
        (numSalesPersonTrees + 1) * sizeof(SalesPersonCustomerTree));
    if (temp == NULL) {
        fprintf(stderr, "Memory reallocation failed\n");
        return NULL;
    }
    salesPersonCustomerTrees = temp;
    strcpy(salesPersonCustomerTrees[numSalesPersonTrees].salesPersonId, salesPersonId);
    salesPersonCustomerTrees[numSalesPersonTrees].customerTree = createBPlusTree(2);
    numSalesPersonTrees++;
    return salesPersonCustomerTrees[numSalesPersonTrees - 1].customerTree;
}

// Conversion functions
char* carTypeToString(CarType type) {
    switch (type) {
        case HATCHBACK: return "Hatchback";
        case SEDAN: return "Sedan";
        case SUV: return "SUV";
        default: return "Unknown";
    }
}

char* fuelTypeToString(FuelType type) {
    switch (type) {
        case PETROL: return "Petrol";
        case DIESEL: return "Diesel";
        case CNG: return "CNG";
        case ELECTRIC: return "Electric";
        case HYBRID: return "Hybrid";
        default: return "Unknown";
    }
}

char* paymentTypeToString(PaymentType type) {
    switch (type) {
        case CASH: return "Cash";
        case LOAN: return "Loan";
        default: return "Unknown";
    }
}

CarType stringToCarType(const char* str) {
    if (strcmp(str, "Hatchback") == 0) return HATCHBACK;
    if (strcmp(str, "Sedan") == 0) return SEDAN;
    if (strcmp(str, "SUV") == 0) return SUV;
    return HATCHBACK; // Default
}

FuelType stringToFuelType(const char* str) {
    if (strcmp(str, "Petrol") == 0) return PETROL;
    if (strcmp(str, "Diesel") == 0) return DIESEL;
    if (strcmp(str, "CNG") == 0) return CNG;
    if (strcmp(str, "Electric") == 0) return ELECTRIC;
    if (strcmp(str, "Hybrid") == 0) return HYBRID;
    return PETROL; // Default
}

PaymentType stringToPaymentType(const char* str) {
    if (strcmp(str, "Cash") == 0) return CASH;
    if (strcmp(str, "Loan") == 0) return LOAN;
    return CASH; // Default
}

double calculateEMI(double principal, double rate, int time) {
    double r = rate / (12 * 100);
    double emi = principal * r * pow(1 + r, time) / (pow(1 + r, time) - 1);
    return emi;
}

void displayCarDetails(Car* car) {
    if (car == NULL) {
        printf("Car not found.\n");
        return;
    }
    printf("VIN: %s\n", car->VIN);
    printf("Name: %s\n", car->name);
    printf("Color: %s\n", car->color);
    printf("Price: %.2f lakhs\n", car->price);
    printf("Fuel Type: %s\n", fuelTypeToString(car->fuelType));
    printf("Car Type: %s\n", carTypeToString(car->carType));
    printf("Status: %s\n", car->isSold ? "Sold" : "Available");
    printf("Showroom ID: %d\n", car->showroomId);
}

void displayCustomerDetails(Customer* customer) {
    if (customer == NULL) {
        printf("Customer not found.\n");
        return;
    }
    printf("Name: %s\n", customer->name);
    printf("Mobile: %s\n", customer->mobileNo);
    printf("Address: %s\n", customer->address);
    printf("Car VIN: %s\n", customer->VIN);
    printf("Registration No: %s\n", customer->registrationNo);
    printf("Payment Type: %s\n", paymentTypeToString(customer->paymentType));
    if (customer->paymentType == LOAN) {
        printf("EMI Months: %d\n", customer->emiMonths);
        printf("Down Payment: %.2f lakhs\n", customer->downPayment);
        printf("Loan Amount: %.2f lakhs\n", customer->loanAmount);
        printf("EMI Amount: %.2f rupees\n", customer->emiAmount);
    }
}

void displaySalesPersonDetails(SalesPerson* person) {
    if (person == NULL) {
        printf("Sales person not found.\n");
        return;
    }
    printf("ID: %d\n", person->id);
    printf("Name: %s\n", person->name);
    printf("Sales Target: %.2f lakhs\n", person->salesTarget);
    printf("Sales Achieved: %.2f lakhs\n", person->salesAchieved);
    printf("Commission (2%%): %.2f lakhs\n", person->commission);
    printf("Number of Sales: %d\n", person->numSales);
    if (person->extraIncentive) {
        printf("Extra Incentive (1%%): %.2f lakhs\n", 0.01 * person->salesAchieved);
        printf("Total Commission: %.2f lakhs\n", 0.03 * person->salesAchieved);
    }
}

void displayShowroomDetails(Showroom* showroom) {
    if (showroom == NULL) {
        printf("Showroom not found.\n");
        return;
    }
    printf("ID: %d\n", showroom->id);
    printf("Name: %s\n", showroom->name);
    printf("Manufacturer: %s\n", showroom->manufacturer);
    printf("Total Cars: %d\n", showroom->numTotalCars);
    printf("Available Cars: %d\n", showroom->numAvailableCars);
    printf("Sold Cars: %d\n", showroom->numSoldCars);
    printf("Total Sales: %.2f lakhs\n", showroom->totalSales);
    printf("Last Month Sales: %.2f lakhs\n", showroom->lastMonthSales);
}

// A. Merge showroom trees
void traverseAndMerge(BPlusTree* mergedTree, BPlusTreeNode* node) {
    if (node == NULL) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            insertIntoBPlusTree(mergedTree, node->keys[i], node->data[i]);
        }
        traverseAndMerge(mergedTree, node->next);
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            traverseAndMerge(mergedTree, node->children[i]);
        }
    }
}

void mergeShowroomTrees(BPlusTree* mergedTree, BPlusTree* showroom1, BPlusTree* showroom2, BPlusTree* showroom3) {
    traverseAndMerge(mergedTree, showroom1->root);
    traverseAndMerge(mergedTree, showroom2->root);
    traverseAndMerge(mergedTree, showroom3->root);
    printf("Merged all three showrooms successfully.\n");
}

// B. Add a new sales person
void addSalesPerson(int showroomId, SalesPerson* person) {
    if (showroomId < 1 || showroomId > MAX_SHOWROOMS || salesPersonTrees[showroomId - 1] == NULL) {
        printf("Invalid or uninitialized showroom ID %d.\n", showroomId);
        return;
    }

    SalesPerson* newPerson = (SalesPerson*)malloc(sizeof(SalesPerson));
    if (newPerson == NULL) {
        fprintf(stderr, "Memory allocation failed for SalesPerson\n");
        return;
    }
    *newPerson = *person;
    newPerson->salesTarget = 50.0;
    newPerson->salesAchieved = 0.0;
    newPerson->commission = 0.0;
    newPerson->numSales = 0;
    newPerson->extraIncentive = false;

    char key[20];
    sprintf(key, "%d", person->id);
    insertIntoBPlusTree(salesPersonTrees[showroomId - 1], key, newPerson);
    printf("Added sales person %s with ID %d to showroom %d.\n", person->name, person->id, showroomId);
    saveSalesPersonsToFile();
}

// C. Find the most popular car
typedef struct {
    char model[MAX_STRING];
    int count;
} ModelCount;

void countModelSales(BPlusTreeNode* node, ModelCount** counts, int* numModels) {
    if (node == NULL) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            Car* car = (Car*)node->data[i];
            bool found = false;
            for (int j = 0; j < *numModels; j++) {
                if (strcmp((*counts)[j].model, car->name) == 0) {
                    (*counts)[j].count++;
                    found = true;
                    break;
                }
            }
            if (!found) {
                (*numModels)++;
                *counts = (ModelCount*)realloc(*counts, (*numModels) * sizeof(ModelCount));
                strcpy((*counts)[(*numModels) - 1].model, car->name);
                (*counts)[(*numModels) - 1].count = 1;
            }
        }
        countModelSales(node->next, counts, numModels);
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            countModelSales(node->children[i], counts, numModels);
        }
    }
}

Car* findCarByModel(BPlusTreeNode* node, const char* mostPopularModel) {
    if (node == NULL) return NULL;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            Car* car = (Car*)node->data[i];
            if (strcmp(car->name, mostPopularModel) == 0) {
                return car;
            }
        }
        return findCarByModel(node->next, mostPopularModel);
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            Car* car = findCarByModel(node->children[i], mostPopularModel);
            if (car != NULL) return car;
        }
    }
    return NULL;
}

Car* findMostPopularCar() {
    ModelCount* counts = (ModelCount*)malloc(sizeof(ModelCount));
    int numModels = 0;
    countModelSales(soldCarTree->root, &counts, &numModels);

    int maxCount = 0;
    char mostPopularModel[MAX_STRING] = "";
    for (int i = 0; i < numModels; i++) {
        if (counts[i].count > maxCount) {
            maxCount = counts[i].count;
            strcpy(mostPopularModel, counts[i].model);
        }
    }
    free(counts);

    Car* mostPopularCar = findCarByModel(carTree->root, mostPopularModel);
    if (mostPopularCar != NULL) {
        printf("The most popular car is %s with %d sales.\n", mostPopularModel, maxCount);
    } else {
        printf("No cars found.\n");
    }
    return mostPopularCar;
}

// D. Find the most successful sales person
void findHighestSales(BPlusTreeNode* node, SalesPerson** mostSuccessful, double* maxSales) {
    if (node == NULL) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            SalesPerson* person = (SalesPerson*)node->data[i];
            if (person->salesAchieved > *maxSales) {
                *maxSales = person->salesAchieved;
                *mostSuccessful = person;
            }
        }
        findHighestSales(node->next, mostSuccessful, maxSales);
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            findHighestSales(node->children[i], mostSuccessful, maxSales);
        }
    }
}

SalesPerson* findMostSuccessfulSalesPerson() {
    SalesPerson* mostSuccessful = NULL;
    double maxSales = 0.0;

    for (int i = 0; i < MAX_SHOWROOMS; i++) {
        if (salesPersonTrees[i] != NULL) {
            findHighestSales(salesPersonTrees[i]->root, &mostSuccessful, &maxSales);
        }
    }

    if (mostSuccessful != NULL) {
        mostSuccessful->extraIncentive = true;
        double extraIncentive = 0.01 * mostSuccessful->salesAchieved;
        printf("The most successful sales person is %s with sales of %.2f lakhs.\n",
               mostSuccessful->name, mostSuccessful->salesAchieved);
        printf("Awarding an extra incentive of %.2f lakhs (1%% of sales).\n", extraIncentive);
        printf("Total commission: %.2f lakhs (2%% standard) + %.2f lakhs (1%% bonus) = %.2f lakhs.\n",
               0.02 * mostSuccessful->salesAchieved, extraIncentive, 0.03 * mostSuccessful->salesAchieved);
    } else {
        printf("No sales persons found.\n");
    }
    return mostSuccessful;
}

// E. Sell a car to a customer
void sellCar(char* salesPersonId, char* VIN, Customer* customer) {
    char showroomIdStr[20], personIdStr[20];
    sscanf(salesPersonId, "%[^_]_%s", showroomIdStr, personIdStr);
    int showroomId = atoi(showroomIdStr);

    if (showroomId < 1 || showroomId > MAX_SHOWROOMS || salesPersonTrees[showroomId - 1] == NULL) {
        printf("Invalid showroom ID in salesPersonId %s.\n", salesPersonId);
        return;
    }

    SalesPerson* salesPerson = (SalesPerson*)searchInBPlusTree(salesPersonTrees[showroomId - 1], personIdStr);
    if (salesPerson == NULL) {
        printf("Sales person with ID %s not found in showroom %d.\n", personIdStr, showroomId);
        return;
    }

    Car* car = (Car*)searchInBPlusTree(carTree, VIN);
    if (car == NULL) {
        printf("Car with VIN %s not found.\n", VIN);
        return;
    }
    if (car->isSold) {
        printf("Car with VIN %s is already sold.\n", VIN);
        return;
    }

    car->isSold = true;
    deleteFromBPlusTree(availableCarTree, VIN);
    insertIntoBPlusTree(soldCarTree, VIN, car);

    Customer* newCustomer = (Customer*)malloc(sizeof(Customer));
    if (newCustomer == NULL) {
        fprintf(stderr, "Memory allocation failed for Customer\n");
        return;
    }
    *newCustomer = *customer;
    strcpy(newCustomer->VIN, VIN);

    newCustomer->loanAmount = 0.0;
    newCustomer->emiAmount = 0.0;
    newCustomer->downPayment = 0.0;
    newCustomer->emiMonths = 0;

    if (customer->paymentType == LOAN) {
        if (customer->downPayment < 0.2 * car->price) {
            printf("Down payment must be at least 20%% of car price (%.2f lakhs).\n", 0.2 * car->price);
            free(newCustomer);
            return;
        }
        newCustomer->downPayment = customer->downPayment;
        newCustomer->loanAmount = car->price - newCustomer->downPayment;
        double rate;
        if (customer->emiMonths == 84) rate = 9.00;
        else if (customer->emiMonths == 60) rate = 8.75;
        else if (customer->emiMonths == 36) rate = 8.50;
        else {
            printf("Invalid EMI duration. Please choose 36, 60, or 84 months.\n");
            free(newCustomer);
            return;
        }
        newCustomer->emiMonths = customer->emiMonths;
        newCustomer->emiAmount = calculateEMI(newCustomer->loanAmount * 100000, rate, newCustomer->emiMonths);
    }

    BPlusTree* customerTree = getCustomerTreeForSalesPerson(salesPersonId);
    insertIntoBPlusTree(customerTree, newCustomer->mobileNo, newCustomer);

    salesPerson->salesAchieved += car->price;
    salesPerson->numSales++;
    salesPerson->commission = 0.02 * salesPerson->salesAchieved;

    Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
    if (showroom != NULL) {
        showroom->numSoldCars++;
        showroom->numAvailableCars--;
        showroom->totalSales += car->price;
        showroom->lastMonthSales += car->price;  // Update last month's sales
        showroom->lastMonthCars++;              // Increment car count for last month
    }

    printf("Car with VIN %s sold to %s for %.2f lakhs.\n", VIN, newCustomer->name, car->price);
    if (newCustomer->paymentType == LOAN) {
        printf("Loan details: Down Payment = %.2f lakhs, Loan Amount = %.2f lakhs, EMI = %.2f rupees for %d months.\n",
               newCustomer->downPayment, newCustomer->loanAmount, newCustomer->emiAmount, newCustomer->emiMonths);
    } else {
        printf("Payment: Cash\n");
    }

    printf("\nCustomer Details After Sale:\n");
    displayCustomerDetails(newCustomer);
    saveCarsToFile();
    saveCustomersToFile();
    saveSalesPersonsToFile();
    saveShowroomsToFile();
}

// F. Predict next month's sales
double predictNextMonthSales(int showroomId) {
    char showroomIdStr[20];
    sprintf(showroomIdStr, "%d", showroomId);
    Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
    if (showroom == NULL) {
        printf("Showroom with ID %d not found.\n", showroomId);
        return 0.0;
    }

    // Assuming current date is April 8, 2025 (as per your example)
    double weightedMean = (0.5 * showroom->lastMonthSales) + 
                         (0.3 * showroom->twoMonthsAgoSales) + 
                         (0.2 * showroom->threeMonthsAgoSales);

    printf("Sales for showroom %s:\n", showroom->name);
    printf("  Last month (2025-03-08 to 2025-04-08): %.2f lakhs (%d cars)\n", 
           showroom->lastMonthSales, showroom->lastMonthCars);
    printf("  Two months ago (2025-02-08 to 2025-03-08): %.2f lakhs (%d cars)\n", 
           showroom->twoMonthsAgoSales, showroom->twoMonthsAgoCars);
    printf("  Three months ago (2025-01-08 to 2025-02-08): %.2f lakhs (%d cars)\n", 
           showroom->threeMonthsAgoSales, showroom->threeMonthsAgoCars);
    printf("Weighted Mean (50%% last, 30%% two months, 20%% three months): %.2f lakhs\n", 
           weightedMean);
    printf("Predicted sales for 2025-05: %.2f lakhs\n", weightedMean);

    return weightedMean;
}
// G. Display all information of a car by VIN
void findCustomerByVIN(BPlusTreeNode* node, char* VIN) {
    if (node == NULL) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            Customer* customer = (Customer*)node->data[i];
            if (strcmp(customer->VIN, VIN) == 0) {
                printf("\nCustomer Information:\n");
                displayCustomerDetails(customer);
                return;
            }
        }
        findCustomerByVIN(node->next, VIN);
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            findCustomerByVIN(node->children[i], VIN);
        }
    }
}

void displayCarByVIN(char* VIN) {
    Car* car = (Car*)searchInBPlusTree(carTree, VIN);
    if (car == NULL) {
        printf("Car with VIN %s not found.\n", VIN);
        return;
    }
    displayCarDetails(car);
    if (car->isSold) {
        for (int i = 0; i < numSalesPersonTrees; i++) {
            findCustomerByVIN(salesPersonCustomerTrees[i].customerTree->root, VIN);
        }
    }
}

// H. Search sales persons within a sales range
void searchSalesPersonByRange(BPlusTree* tree, double minSales, double maxSales) {
    if (tree == NULL || tree->root == NULL) {
        printf("No sales persons found in database.\n");
        return;
    }
    printf("\n--- Sales Persons with Sales Achievement between %.2f and %.2f lakhs ---\n", minSales, maxSales);
    BPlusTreeNode* current = tree->root;
    while (!current->isLeaf) {
        current = current->children[0];
    }
    bool found = false;
    while (current != NULL) {
        for (int i = 0; i < current->numKeys; i++) {
            SalesPerson* salesPerson = (SalesPerson*)current->data[i];
            if (salesPerson->salesAchieved >= minSales && salesPerson->salesAchieved <= maxSales) {
                printf("ID: %d, Name: %s, Sales Achieved: %.2f lakhs\n",
                       salesPerson->id, salesPerson->name, salesPerson->salesAchieved);
                found = true;
            }
        }
        current = current->next;
    }
    if (!found) {
        printf("No sales persons found in the specified range.\n");
    }
}

// I. Print customers with EMI in range 36-48 months
void findCustomersWithEMIInRange(BPlusTreeNode* node, int* count) {
    if (node == NULL) return;
    if (node->isLeaf) {
        for (int i = 0; i < node->numKeys; i++) {
            Customer* customer = (Customer*)node->data[i];
            if (customer->paymentType == LOAN && customer->emiMonths >= 36 && customer->emiMonths <= 48) {
                printf("%d. %s - Mobile: %s, EMI: %d months, Amount: %.2f rupees\n",
                       ++(*count), customer->name, customer->mobileNo, customer->emiMonths, customer->emiAmount);
            }
        }
        findCustomersWithEMIInRange(node->next, count);
    } else {
        for (int i = 0; i <= node->numKeys; i++) {
            findCustomersWithEMIInRange(node->children[i], count);
        }
    }
}

void printCustomersWithEMIInRange() {
    printf("Customers with EMI plan between 36 and 48 months:\n");
    int count = 0;
    for (int i = 0; i < numSalesPersonTrees; i++) {
        findCustomersWithEMIInRange(salesPersonCustomerTrees[i].customerTree->root, &count);
    }
    if (count == 0) {
        printf("No customers found with EMI between 36 and 48 months.\n");
    }
}

// Deletion functions
int finddeleteKeyPosition(BPlusTreeNode* node, char* key) {
    int idx = 0;
    while (idx < node->numKeys && strcmp(key, node->keys[idx]) > 0) {
        idx++;
    }
    return idx;
}

char* getPredecessor(BPlusTreeNode* node, int idx) {
    BPlusTreeNode* curr = node->children[idx];
    while (!curr->isLeaf) {
        curr = curr->children[curr->numKeys];
    }
    return curr->keys[curr->numKeys - 1];
}

void removeFromLeaf(BPlusTreeNode* node, int idx) {
    for (int i = idx + 1; i < node->numKeys; i++) {
        strcpy(node->keys[i - 1], node->keys[i]);
        node->data[i - 1] = node->data[i];
    }
    node->numKeys--;
}

void borrowFromPrev(BPlusTreeNode* parent, int idx) {
    BPlusTreeNode* child = parent->children[idx];
    BPlusTreeNode* sibling = parent->children[idx - 1];
    for (int i = child->numKeys - 1; i >= 0; i--) {
        strcpy(child->keys[i + 1], child->keys[i]);
        child->data[i + 1] = child->data[i];
    }
    if (!child->isLeaf) {
        for (int i = child->numKeys; i >= 0; i--) {
            child->children[i + 1] = child->children[i];
        }
    }
    strcpy(child->keys[0], parent->keys[idx - 1]);
    child->data[0] = parent->data[idx - 1];
    if (!child->isLeaf) {
        child->children[0] = sibling->children[sibling->numKeys];
    }
    strcpy(parent->keys[idx - 1], sibling->keys[sibling->numKeys - 1]);
    parent->data[idx - 1] = sibling->data[sibling->numKeys - 1];
    child->numKeys++;
    sibling->numKeys--;
}

void borrowFromNext(BPlusTreeNode* parent, int idx) {
    BPlusTreeNode* child = parent->children[idx];
    BPlusTreeNode* sibling = parent->children[idx + 1];
    strcpy(child->keys[child->numKeys], parent->keys[idx]);
    child->data[child->numKeys] = parent->data[idx];
    if (!child->isLeaf) {
        child->children[child->numKeys + 1] = sibling->children[0];
    }
    strcpy(parent->keys[idx], sibling->keys[0]);
    parent->data[idx] = sibling->data[0];
    for (int i = 1; i < sibling->numKeys; i++) {
        strcpy(sibling->keys[i - 1], sibling->keys[i]);
        sibling->data[i - 1] = sibling->data[i];
    }
    if (!sibling->isLeaf) {
        for (int i = 1; i <= sibling->numKeys; i++) {
            sibling->children[i - 1] = sibling->children[i];
        }
    }
    child->numKeys++;
    sibling->numKeys--;
}

void merge(BPlusTreeNode* parent, int idx) {
    BPlusTreeNode* child = parent->children[idx];
    BPlusTreeNode* sibling = parent->children[idx + 1];
    strcpy(child->keys[child->numKeys], parent->keys[idx]);
    child->data[child->numKeys] = parent->data[idx];
    for (int i = 0; i < sibling->numKeys; i++) {
        strcpy(child->keys[child->numKeys + 1 + i], sibling->keys[i]);
        child->data[child->numKeys + 1 + i] = sibling->data[i];
    }
    if (!child->isLeaf) {
        for (int i = 0; i <= sibling->numKeys; i++) {
            child->children[child->numKeys + 1 + i] = sibling->children[i];
        }
    }
    for (int i = idx + 1; i < parent->numKeys; i++) {
        strcpy(parent->keys[i - 1], parent->keys[i]);
        parent->data[i - 1] = parent->data[i];
    }
    for (int i = idx + 2; i <= parent->numKeys; i++) {
        parent->children[i - 1] = parent->children[i];
    }
    child->numKeys += sibling->numKeys + 1;
    parent->numKeys--;
    free(sibling);
}

void fill(BPlusTreeNode* parent, int idx, int minKeys) {
    if (idx != 0 && parent->children[idx - 1]->numKeys > minKeys) {
        borrowFromPrev(parent, idx);
    } else if (idx != parent->numKeys && parent->children[idx + 1]->numKeys > minKeys) {
        borrowFromNext(parent, idx);
    } else {
        if (idx != parent->numKeys) {
            merge(parent, idx);
        } else {
            merge(parent, idx - 1);
        }
    }
}

void deleteKeyHelper(BPlusTreeNode* node, char* key, int minKeys) {
    int idx = finddeleteKeyPosition(node, key);
    if (idx < node->numKeys && strcmp(node->keys[idx], key) == 0) {
        if (node->isLeaf) {
            removeFromLeaf(node, idx);
        } else {
            char* predecessor = getPredecessor(node, idx);
            strcpy(node->keys[idx], predecessor);
            BPlusTreeNode* predNode = node->children[idx];
            while (!predNode->isLeaf) {
                predNode = predNode->children[predNode->numKeys];
            }
            node->data[idx] = predNode->data[predNode->numKeys - 1];
            deleteKeyHelper(node->children[idx], predecessor, minKeys);
        }
    } else {
        if (node->isLeaf) {
            printf("Key %s not found in the B+ tree.\n", key);
            return;
        }
        bool isLastChild = (idx == node->numKeys);
        if (node->children[idx]->numKeys <= minKeys) {
            fill(node, idx, minKeys);
        }
        if (isLastChild && idx > node->numKeys) {
            deleteKeyHelper(node->children[idx - 1], key, minKeys);
        } else {
            deleteKeyHelper(node->children[idx], key, minKeys);
        }
    }
}

void deleteFromBPlusTree(BPlusTree* tree, char* key) {
    if (tree->root == NULL) {
        return;
    }
    int minKeys = (tree->order / 2) - 1;
    deleteKeyHelper(tree->root, key, minKeys);
    if (tree->root->numKeys == 0 && !tree->root->isLeaf) {
        BPlusTreeNode* oldRoot = tree->root;
        tree->root = tree->root->children[0];
        free(oldRoot);
    }
    if (tree->root->numKeys == 0 && tree->root->isLeaf) {
        BPlusTreeNode* oldRoot = tree->root;
        tree->root = NULL;
        free(oldRoot);
    }
}

void printNode(BPlusTreeNode* node, int level) {
    if (node == NULL) return;
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("Keys: ");
    for (int i = 0; i < node->numKeys; i++) {
        printf("%s ", node->keys[i]);
    }
    printf("\n");
    if (!node->isLeaf) {
        for (int i = 0; i <= node->numKeys; i++) {
            printNode(node->children[i], level + 1);
        }
    }
}

void printBPlusTree(BPlusTree* tree) {
    if (tree->root == NULL) {
        printf("Empty tree.\n");
        return;
    }
    printNode(tree->root, 0);
}

void saveCarsToFile() {
    FILE* fp = fopen("cars.txt", "w");
    if (!fp) {
        printf("Error opening cars.txt for writing.\n");
        return;
    }
    BPlusTreeNode* current = carTree->root;
    while (current && !current->isLeaf) current = current->children[0];
    while (current) {
        for (int i = 0; i < current->numKeys; i++) {
            Car* car = (Car*)current->data[i];
            fprintf(fp, "%s,%s,%s,%.2f,%d,%d,%d,%d\n",
                    car->VIN, car->name, car->color, car->price,
                    car->fuelType, car->carType, car->isSold, car->showroomId);
        }
        current = current->next;
    }
    fclose(fp);
}

void loadCarsFromFile() {
    FILE* fp = fopen("cars.txt", "r");
    if (!fp) return;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        Car* car = (Car*)malloc(sizeof(Car));
        int isSold;
        sscanf(line, "%19[^,],%99[^,],%99[^,],%lf,%d,%d,%d,%d",
               car->VIN, car->name, car->color, &car->price,
               (int*)&car->fuelType, (int*)&car->carType, &isSold, &car->showroomId);
        car->isSold = (bool)isSold;
        insertIntoBPlusTree(carTree, car->VIN, car);
        if (car->isSold)
            insertIntoBPlusTree(soldCarTree, car->VIN, car);
        else
            insertIntoBPlusTree(availableCarTree, car->VIN, car);
    }
    fclose(fp);
}

void saveCustomersToFile() {
    FILE* fp = fopen("customers.txt", "w");
    if (!fp) {
        printf("Error opening customers.txt for writing.\n");
        return;
    }
    for (int i = 0; i < numSalesPersonTrees; i++) {
        BPlusTreeNode* current = salesPersonCustomerTrees[i].customerTree->root;
        while (current && !current->isLeaf) current = current->children[0];
        while (current) {
            for (int j = 0; j < current->numKeys; j++) {
                Customer* customer = (Customer*)current->data[j];
                fprintf(fp, "%s,%s,%s,%s,%s,%s,%d,%d,%.2f,%.2f,%.2f\n",
                        salesPersonCustomerTrees[i].salesPersonId, customer->name, customer->mobileNo,
                        customer->address, customer->VIN, customer->registrationNo,
                        customer->paymentType, customer->emiMonths, customer->downPayment,
                        customer->loanAmount, customer->emiAmount);
            }
            current = current->next;
        }
    }
    fclose(fp);
}

void loadCustomersFromFile() {
    FILE* fp = fopen("customers.txt", "r");
    if (!fp) return;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        Customer* customer = (Customer*)malloc(sizeof(Customer));
        char salesPersonId[50];
        sscanf(line, "%49[^,],%99[^,],%14[^,],%99[^,],%19[^,],%19[^,],%d,%d,%lf,%lf,%lf",
               salesPersonId, customer->name, customer->mobileNo, customer->address,
               customer->VIN, customer->registrationNo, (int*)&customer->paymentType,
               &customer->emiMonths, &customer->downPayment, &customer->loanAmount,
               &customer->emiAmount);
        BPlusTree* customerTree = getCustomerTreeForSalesPerson(salesPersonId);
        insertIntoBPlusTree(customerTree, customer->mobileNo, customer);
    }
    fclose(fp);
}

void saveSalesPersonsToFile() {
    FILE* fp = fopen("salespersons.txt", "w");
    if (!fp) {
        printf("Error opening salespersons.txt for writing.\n");
        return;
    }
    for (int i = 0; i < MAX_SHOWROOMS; i++) {
        if (salesPersonTrees[i]) {
            BPlusTreeNode* current = salesPersonTrees[i]->root;
            while (current && !current->isLeaf) current = current->children[0];
            while (current) {
                for (int j = 0; j < current->numKeys; j++) {
                    SalesPerson* person = (SalesPerson*)current->data[j];
                    fprintf(fp, "%d,%d,%s,%.2f,%.2f,%.2f,%d,%d\n",
                            i + 1, person->id, person->name, person->salesTarget,
                            person->salesAchieved, person->commission, person->numSales,
                            person->extraIncentive);
                }
                current = current->next;
            }
        }
    }
    fclose(fp);
}

void loadSalesPersonsFromFile() {
    FILE* fp = fopen("salespersons.txt", "r");
    if (!fp) return;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        SalesPerson* person = (SalesPerson*)malloc(sizeof(SalesPerson));
        int showroomId, extraIncentive;
        sscanf(line, "%d,%d,%99[^,],%lf,%lf,%lf,%d,%d",
               &showroomId, &person->id, person->name, &person->salesTarget,
               &person->salesAchieved, &person->commission, &person->numSales,
               &extraIncentive);
        person->extraIncentive = (bool)extraIncentive;
        char key[20];
        sprintf(key, "%d", person->id);
        insertIntoBPlusTree(salesPersonTrees[showroomId - 1], key, person);
    }
    fclose(fp);
}

void saveShowroomsToFile() {
    FILE* fp = fopen("showrooms.txt", "w");
    if (!fp) {
        printf("Error opening showrooms.txt for writing.\n");
        return;
    }
    BPlusTreeNode* current = showroomTree->root;
    while (current && !current->isLeaf) current = current->children[0];
    while (current) {
        for (int i = 0; i < current->numKeys; i++) {
            Showroom* showroom = (Showroom*)current->data[i];
            fprintf(fp, "%d,%s,%s,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%d,%d,%d\n",
                    showroom->id, showroom->name, showroom->manufacturer,
                    showroom->numTotalCars, showroom->numAvailableCars,
                    showroom->numSoldCars, showroom->totalSales, 
                    showroom->lastMonthSales, showroom->twoMonthsAgoSales, 
                    showroom->threeMonthsAgoSales, showroom->lastMonthCars,
                    showroom->twoMonthsAgoCars, showroom->threeMonthsAgoCars);
        }
        current = current->next;
    }
    fclose(fp);
}

void loadShowroomsFromFile() {
    FILE* fp = fopen("showrooms.txt", "r");
    if (!fp) return;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        Showroom* showroom = (Showroom*)malloc(sizeof(Showroom));
        sscanf(line, "%d,%99[^,],%99[^,],%d,%d,%d,%lf,%lf,%lf,%lf,%d,%d,%d",
               &showroom->id, showroom->name, showroom->manufacturer,
               &showroom->numTotalCars, &showroom->numAvailableCars,
               &showroom->numSoldCars, &showroom->totalSales, 
               &showroom->lastMonthSales, &showroom->twoMonthsAgoSales, 
               &showroom->threeMonthsAgoSales, &showroom->lastMonthCars,
               &showroom->twoMonthsAgoCars, &showroom->threeMonthsAgoCars);
        char key[20];
        sprintf(key, "%d", showroom->id);
        insertIntoBPlusTree(showroomTree, key, showroom);
    }
    fclose(fp);
}

void addCar(int showroomId, Car* car) {
    if (showroomId < 1 || showroomId > MAX_SHOWROOMS) {
        printf("Invalid showroom ID %d. Must be between 1 and %d.\n", showroomId, MAX_SHOWROOMS);
        return;
    }

    if (searchInBPlusTree(carTree, car->VIN) != NULL) {
        printf("Car with VIN %s already exists.\n", car->VIN);
        return;
    }

    Car* newCar = (Car*)malloc(sizeof(Car));
    if (newCar == NULL) {
        fprintf(stderr, "Memory allocation failed for Car\n");
        return;
    }
    *newCar = *car;
    newCar->isSold = false;
    newCar->showroomId = showroomId;

    insertIntoBPlusTree(carTree, newCar->VIN, newCar);
    insertIntoBPlusTree(availableCarTree, newCar->VIN, newCar);

    char showroomIdStr[20];
    sprintf(showroomIdStr, "%d", showroomId);
    Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
    if (showroom != NULL) {
        showroom->numTotalCars++;
        showroom->numAvailableCars++;
    }

    printf("Added car %s with VIN %s to showroom %d.\n", newCar->name, newCar->VIN, showroomId);
    saveCarsToFile();
    saveShowroomsToFile();
}

void mergeAndSortShowroomsByVIN(const char* outputName) {
    ShowroomCar* allCars = NULL;
    int totalCars = 0;
    
    BPlusTreeNode* current = carTree->root;
    while (current && !current->isLeaf) current = current->children[0];
    
    while (current != NULL) {
        for (int i = 0; i < current->numKeys; i++) {
            ShowroomCar* temp = realloc(allCars, (totalCars + 1) * sizeof(ShowroomCar));
            if (temp == NULL) {
                printf("Memory allocation failed\n");
                free(allCars);
                return;
            }
            allCars = temp;
            allCars[totalCars].car = (Car*)current->data[i];
            allCars[totalCars].showroomId = allCars[totalCars].car->showroomId;
            totalCars++;
        }
        current = current->next;
    }
    
    for (int i = 0; i < totalCars - 1; i++) {
        for (int j = 0; j < totalCars - i - 1; j++) {
            if (strcmp(allCars[j].car->VIN, allCars[j + 1].car->VIN) > 0) {
                ShowroomCar temp = allCars[j];
                allCars[j] = allCars[j + 1];
                allCars[j + 1] = temp;
            }
        }
    }
    
    printf("\n=== Merged and Sorted Cars from All Showrooms (%s) ===\n", outputName);
    printf("VIN\t\tName\t\tColor\tPrice\tFuel Type\tCar Type\tStatus\tShowroom ID\n");
    printf("----------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < totalCars; i++) {
        Car* car = allCars[i].car;
        printf("%-16s%-16s%-8s%-8.2f%-12s%-12s%-8s%d\n",
               car->VIN, car->name, car->color, car->price,
               fuelTypeToString(car->fuelType), carTypeToString(car->carType),
               car->isSold ? "Sold" : "Available", allCars[i].showroomId);
    }
    
    FILE* fp = fopen(outputName, "w");
    if (fp == NULL) {
        printf("Error opening file %s for writing\n", outputName);
    } else {
        fprintf(fp, "VIN,Name,Color,Price,Fuel Type,Car Type,Status,Showroom ID\n");
        for (int i = 0; i < totalCars; i++) {
            Car* car = allCars[i].car;
            fprintf(fp, "%s,%s,%s,%.2f,%s,%s,%s,%d\n",
                    car->VIN, car->name, car->color, car->price,
                    fuelTypeToString(car->fuelType), carTypeToString(car->carType),
                    car->isSold ? "Sold" : "Available", allCars[i].showroomId);
        }
        fclose(fp);
        printf("\nMerged data saved to %s\n", outputName);
    }
    
    free(allCars);
}

// New functions
void displayAllCarsShowroomWise() {
    printf("\n=== Cars Organized by Showroom ===\n");
    for (int i = 1; i <= MAX_SHOWROOMS; i++) {
        char showroomIdStr[20];
        sprintf(showroomIdStr, "%d", i);
        Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
        
        if (showroom == NULL) {
            printf("Showroom %d not found.\n", i);
            continue;
        }
        
        printf("\nShowroom %d: %s (%s)\n", showroom->id, showroom->name, showroom->manufacturer);
        printf("----------------------------------------\n");
        printf("VIN\t\tName\t\tColor\tPrice\tStatus\n");
        printf("----------------------------------------\n");
        
        int carCount = 0;
        BPlusTreeNode* current = carTree->root;
        while (current && !current->isLeaf) current = current->children[0];
        
        while (current != NULL) {
            for (int j = 0; j < current->numKeys; j++) {
                Car* car = (Car*)current->data[j];
                if (car->showroomId == i) {
                    printf("%-16s%-16s%-8s%-8.2f%s\n",
                           car->VIN, car->name, car->color, car->price,
                           car->isSold ? "Sold" : "Available");
                    carCount++;
                }
            }
            current = current->next;
        }
        
        if (carCount == 0) {
            printf("No cars found in this showroom.\n");
        }
        printf("Total cars: %d\n", carCount);
    }
}

/*void displayAllSalesPersonsShowroomWise() {
    printf("\n=== Salespersons Organized by Showroom ===\n");
    for (int i = 0; i < MAX_SHOWROOMS; i++) {
        char showroomIdStr[20];
        sprintf(showroomIdStr, "%d", i + 1);
        Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
        
        if (showroom == NULL || salesPersonTrees[i] == NULL) {
            printf("Showroom %d not found or not initialized.\n", i + 1);
            continue;
        }
        
        printf("\nShowroom %d: %s\n", i + 1, showroom->name);
        printf("----------------------------------------\n");
        printf("ID\tName\t\tSales Achieved\tCommission\n");
        printf("----------------------------------------\n");
        
        int personCount = 0;
        BPlusTreeNode* current = salesPersonTrees[i]->root;
        while (current && !current->isLeaf) current = current->children[0];
        
        while (current != NULL) {
            for (int j = 0; j < current->numKeys; j++) {
                SalesPerson* person = (SalesPerson*)current->data[j];
                printf("%-8d%-16s%-16.2f%.2f\n",
                       person->id, person->name, person->salesAchieved, Ascending(person->commission));
                personCount++;
            }
            current = current->next;
        }
        
        if (personCount == 0) {
            printf("No salespersons found in this showroom.\n");
        }
        printf("Total salespersons: %d\n", personCount);
    }
}*/
void displayAllSalesPersonsShowroomWise() {
    printf("\n=== Salespersons Organized by Showroom ===\n");
    for (int i = 0; i < MAX_SHOWROOMS; i++) {
        char showroomIdStr[20];
        sprintf(showroomIdStr, "%d", i + 1);
        Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
        
        if (showroom == NULL || salesPersonTrees[i] == NULL) {
            printf("Showroom %d not found or not initialized.\n", i + 1);
            continue;
        }
        
        printf("\nShowroom %d: %s\n", i + 1, showroom->name);
        printf("----------------------------------------\n");
        printf("ID\tName\t\tSales Achieved\tCommission\n");
        printf("----------------------------------------\n");
        
        int personCount = 0;
        BPlusTreeNode* current = salesPersonTrees[i]->root;
        while (current && !current->isLeaf) current = current->children[0];
        
        while (current != NULL) {
            for (int j = 0; j < current->numKeys; j++) {
                SalesPerson* person = (SalesPerson*)current->data[j];
                printf("%-8d%-16s%-16.2f%.2f\n",
                       person->id, person->name, person->salesAchieved, person->commission);
                personCount++;
            }
            current = current->next;
        }
        
        if (personCount == 0) {
            printf("No salespersons found in this showroom.\n");
        }
        printf("Total salespersons: %d\n", personCount);
    }
}

void displayCustomersForSalesPerson(int showroomId, int salesPersonId) {
    char salesPersonIdStr[50];
    sprintf(salesPersonIdStr, "%d_%d", showroomId, salesPersonId);
    
    BPlusTree* customerTree = getCustomerTreeForSalesPerson(salesPersonIdStr);
    if (customerTree == NULL || customerTree->root == NULL) {
        printf("No customer tree found for salesperson %d in showroom %d.\n", 
               salesPersonId, showroomId);
        return;
    }
    
    printf("\n=== Customers for Salesperson %d in Showroom %d ===\n", 
           salesPersonId, showroomId);
    printf("----------------------------------------\n");
    printf("Name\t\tMobile\t\tCar VIN\t\tPayment\n");
    printf("----------------------------------------\n");
    
    int customerCount = 0;
    BPlusTreeNode* current = customerTree->root;
    while (current && !current->isLeaf) current = current->children[0];
    
    while (current != NULL) {
        for (int i = 0; i < current->numKeys; i++) {
            Customer* customer = (Customer*)current->data[i];
            printf("%-16s%-16s%-16s%s\n",
                   customer->name, customer->mobileNo, customer->VIN,
                   paymentTypeToString(customer->paymentType));
            customerCount++;
        }
        current = current->next;
    }
    
    if (customerCount == 0) {
        printf("No customers found for this salesperson.\n");
    }
    printf("Total customers: %d\n", customerCount);
}

// Main function
int main() {
    carTree = createBPlusTree(1);
    availableCarTree = createBPlusTree(1);
    soldCarTree = createBPlusTree(1);
    showroomTree = createBPlusTree(4);

    for (int i = 0; i < MAX_SHOWROOMS; i++) {
        salesPersonTrees[i] = createBPlusTree(3);
    }

    loadShowroomsFromFile();
    loadCarsFromFile();
    loadSalesPersonsFromFile();
    loadCustomersFromFile();

    if (showroomTree->root == NULL) {
        Showroom showroom1 = {1, "Maruti Suzuki Showroom", "Maruti Suzuki", 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0};
        Showroom showroom2 = {2, "Hyundai Showroom", "Hyundai", 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0};
        Showroom showroom3 = {3, "Tata Motors Showroom", "Tata Motors", 0, 0, 0, 0.0, 0.0, 0.0, 0.0, 0, 0, 0};

        char showroomId1[20], showroomId2[20], showroomId3[20];
        sprintf(showroomId1, "%d", showroom1.id);
        sprintf(showroomId2, "%d", showroom2.id);
        sprintf(showroomId3, "%d", showroom3.id);

        Showroom* s1 = (Showroom*)malloc(sizeof(Showroom));
        Showroom* s2 = (Showroom*)malloc(sizeof(Showroom));
        Showroom* s3 = (Showroom*)malloc(sizeof(Showroom));
        *s1 = showroom1;
        *s2 = showroom2;
        *s3 = showroom3;

        insertIntoBPlusTree(showroomTree, showroomId1, s1);
        insertIntoBPlusTree(showroomTree, showroomId2, s2);
        insertIntoBPlusTree(showroomTree, showroomId3, s3);

        saveShowroomsToFile();
    }

    int choice = 0;
    while (choice != 17) {
        printf("\n===== Car Showroom Management System =====\n");
        printf("1. Add New Car\n");
        printf("2. Add Sales Person\n");
        printf("3. Display Showroom Details\n");
        printf("4. Display Car Details\n");
        printf("5. Display Sales Person Details\n");
        printf("6. Sell Car to Customer\n");
        printf("7. Find Most Popular Car\n");
        printf("8. Find Most Successful Sales Person\n");
        printf("9. Predict Next Month Sales\n");
        printf("10. Print Customers with EMI Between 36-48 Months\n");
        printf("11. Search Sales Persons by Sales Range\n");
        printf("12. Merge All Showroom Trees\n");
        printf("13. Merge and Sort All Showrooms by VIN\n");
        printf("14. Display All Cars Showroom-wise\n");
        printf("15. Display All Salespersons Showroom-wise\n");
        printf("16. Display Customers for Specific Salesperson\n");
        printf("17. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                Car newCar;
                int showroomId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                printf("Enter Car VIN: ");
                scanf("%s", newCar.VIN);
                printf("Enter Car Name: ");
                scanf(" %[^\n]", newCar.name);
                printf("Enter Car Color: ");
                scanf(" %[^\n]", newCar.color);
                printf("Enter Car Price (in lakhs): ");
                scanf("%lf", &newCar.price);
                printf("Enter Fuel Type (0-Petrol, 1-Diesel, 2-CNG, 3-Electric, 4-Hybrid): ");
                int fuelType;
                scanf("%d", &fuelType);
                if (fuelType < 0 || fuelType > 4) {
                    printf("Invalid fuel type. Setting to Petrol.\n");
                    newCar.fuelType = PETROL;
                } else {
                    newCar.fuelType = (FuelType)fuelType;
                }
                printf("Enter Car Type (0-Hatchback, 1-Sedan, 2-SUV): ");
                int carType;
                scanf("%d", &carType);
                if (carType < 0 || carType > 2) {
                    printf("Invalid car type. Setting to Hatchback.\n");
                    newCar.carType = HATCHBACK;
                } else {
                    newCar.carType = (CarType)carType;
                }
                addCar(showroomId, &newCar);
                break;
            }
            case 2: {
                SalesPerson newPerson;
                int showroomId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                printf("Enter Sales Person ID: ");
                scanf("%d", &newPerson.id);
                printf("Enter Sales Person Name: ");
                scanf(" %[^\n]", newPerson.name);
                addSalesPerson(showroomId, &newPerson);
                break;
            }
            case 3: {
                int showroomId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                char showroomIdStr[20];
                sprintf(showroomIdStr, "%d", showroomId);
                Showroom* showroom = (Showroom*)searchInBPlusTree(showroomTree, showroomIdStr);
                displayShowroomDetails(showroom);
                break;
            }
            case 4: {
                char VIN[20];
                printf("Enter Car VIN: ");
                scanf("%s", VIN);
                displayCarByVIN(VIN);
                break;
            }
            case 5: {
                int showroomId, personId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                printf("Enter Sales Person ID: ");
                scanf("%d", &personId);
                char personIdStr[20];
                sprintf(personIdStr, "%d", personId);
                SalesPerson* person = (SalesPerson*)searchInBPlusTree(salesPersonTrees[showroomId - 1], personIdStr);
                displaySalesPersonDetails(person);
                break;
            }
            case 6: {
                char salesPersonId[50];
                char VIN[20];
                Customer newCustomer;
                int showroomId, personId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                printf("Enter Sales Person ID: ");
                scanf("%d", &personId);
                sprintf(salesPersonId, "%d_%d", showroomId, personId);
                printf("Enter Car VIN: ");
                scanf("%s", VIN);
                printf("Enter Customer Name: ");
                scanf(" %[^\n]", newCustomer.name);
                printf("Enter Customer Mobile: ");
                scanf("%s", newCustomer.mobileNo);
                printf("Enter Customer Address: ");
                scanf(" %[^\n]", newCustomer.address);
                printf("Enter Car Registration Number: ");
                scanf("%s", newCustomer.registrationNo);
                int paymentType;
                printf("Enter Payment Type (0 for Cash, 1 for Loan): ");
                scanf("%d", &paymentType);
                newCustomer.paymentType = (PaymentType)paymentType;
                if (newCustomer.paymentType == LOAN) {
                    printf("Enter Down Payment (in lakhs): ");
                    scanf("%lf", &newCustomer.downPayment);
                    printf("Enter EMI Duration (36, 60, or 84 months): ");
                    scanf("%d", &newCustomer.emiMonths);
                }
                sellCar(salesPersonId, VIN, &newCustomer);
                break;
            }
            case 7: {
                Car* popularCar = findMostPopularCar();
                if (popularCar != NULL) {
                    printf("\nMost Popular Car Details:\n");
                    displayCarDetails(popularCar);
                }
                break;
            }
            case 8: {
                findMostSuccessfulSalesPerson();
                break;
            }
            case 9: {
                int showroomId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                predictNextMonthSales(showroomId);
                break;
            }
            case 10: {
                printCustomersWithEMIInRange();
                break;
            }
            case 11: {
                int showroomId;
                double minSales, maxSales;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                if (showroomId < 1 || showroomId > MAX_SHOWROOMS || salesPersonTrees[showroomId - 1] == NULL) {
                    printf("Invalid or uninitialized showroom ID %d.\n", showroomId);
                    break;
                }
                printf("Enter minimum sales value (in lakhs): ");
                scanf("%lf", &minSales);
                printf("Enter maximum sales value (in lakhs): ");
                scanf("%lf", &maxSales);
                searchSalesPersonByRange(salesPersonTrees[showroomId - 1], minSales, maxSales);
                break;
            }
            case 12: {
                BPlusTree* mergedTree = createBPlusTree(4);
                mergeShowroomTrees(mergedTree, salesPersonTrees[0], salesPersonTrees[1], salesPersonTrees[2]);
                break;
            }
            case 13: {
                char outputName[MAX_STRING];
                printf("Enter output filename for merged data (e.g., merged_cars.txt): ");
                scanf("%s", outputName);
                mergeAndSortShowroomsByVIN(outputName);
                break;
            }
            case 14: {
                displayAllCarsShowroomWise();
                break;
            }
            case 15: {
                displayAllSalesPersonsShowroomWise();
                break;
            }
            case 16: {
                int showroomId, salesPersonId;
                printf("Enter Showroom ID (1-3): ");
                scanf("%d", &showroomId);
                printf("Enter Sales Person ID: ");
                scanf("%d", &salesPersonId);
                displayCustomersForSalesPerson(showroomId, salesPersonId);
                break;
            }
            case 17: {
                printf("Saving data and exiting...\n");
                saveCarsToFile();
                saveCustomersToFile();
                saveSalesPersonsToFile();
                saveShowroomsToFile();
                printf("Exiting the system. Thank you!\n");
                break;
            }
            default: {
                printf("Invalid choice. Please try again.\n");
                break;
            }
        }
    }

    free(carTree);
    free(availableCarTree);
    free(soldCarTree);
    for (int i = 0; i < numSalesPersonTrees; i++) {
        free(salesPersonCustomerTrees[i].customerTree);
    }
    free(salesPersonCustomerTrees);
    for (int i = 0; i < MAX_SHOWROOMS; i++) {
        if (salesPersonTrees[i] != NULL) {
            free(salesPersonTrees[i]);
        }
    }
    free(showroomTree);

    return 0;
}