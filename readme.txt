Execution flow:
breakTrecFile.cpp
    reads the trec file and breaks into 2142 files of 1500 documents each.
    These files are stored in the data folder.
    naming convention: output_0, output_1

createPostingsForChumls.cpp
    fetches all the files from the data folder
    creates postings for each file in the folder
    All files are stored in the unfilteredPostings folder
    output_0 will have a postings_0 file
    output_1 will have a postings_1 file 

createSubPostingsFiles.cpp
    fetches all the files from the unfilteredPostings folder
    performs min heap and merges all the postings alphabetically
    stores all the files in the sortedPostingsAlphabetically folder
    All words starting with 'a' will be present in postings_a file
    All words starting with 'b' will be present in postings_b file

updateSubPostingsFile.cpp
    fetches data from the sortedPostingsAlphabetically folder
    merges all the similar words into 1 posting 
    stores the data back into those respective files itself
    No new files are formed

mergeSubpostings.cpp
    fetched data from the sortedPostingsAlphabetically folder
    merges all to one file postings_inverted.txt and saves this in the mergePostings folder
    Has all the inverted indexes

    Creates the hashtable from the postings_inverted file in the mergePostings folder
    Saves it in the hashtable folder in a hashtable.txt file. 
    It stores data in this form:
    word, frequency - line number 


compression.cpp
    picks files from the mergePostings folder
    does varByte encoding on all the numbers 
    stores these in .bin files in the compressed file


search.cpp
    reads data from wordPositions/postings_inverted_word_positions.txt file to get the pointers to respective word's inverted indexes
    reads data from docIDToUrlMapping.txt to get the docId and URL docIDToUrlMapping
    reads data from documentPositions/documentPositions.txt to get the pointers to the start of each document based on the docId

    Takes user input
    Performs a disjunctive and conjunctive query Execution
    For any word given, fetches the inverted index position from the first file
    calculates a bm25 score
    Uses the second file to get the associcated URL
    users the third file to get the associcated document to generate the snippet
    returns the top 10 best urls based on the bm25 scores
  
Relevant folders:
    data -> unfilteredPostings, documentPositions -> sortedPostingsAlphabetically -> sortedPostingsAlphabetically -> mergePostings -> hashtable -> compressed, wordPositions
