The ACS program simulates four clerks with two queues (business class and economy class). The clerks will signal customers in business class before customers in the economy class. 

How to use ACS:
Step 1: execute "make" in terminal to compile ACS
Step 2: execute "./ACS" with input file in terminal to run ACS (./ACS input2.txt)

Input file:
The input file is a text file and has a simple format. The first line contains the total number of customers that will be simulated. After that, each line contains the information about a single customer, such that:1. The first character specifies the unique ID of customers.2. A colon(:) immediately follows the unique number of the customer.3. Immediately following is an integer equal to either 1 (indicating the customer belongs to business class) or 0 (indicating the customer belongs to economy class).4. A comma(,) immediately follows the previous number.5. Immediately following is an integer that indicates the arrival time of the customer.6. A comma(,) immediately follows the previous number.7. Immediately following is an integer that indicates the service time of the customer.8. A newline (\n) ends a line.