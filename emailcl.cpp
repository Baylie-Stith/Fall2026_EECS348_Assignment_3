// File Name: ceo_email_priority.cpp
// Author: Baylie Stith
//
// Description: Prioritizes a CEO's incoming emails using a MaxHeap-based priority queue.
//
// Contributers: None
// AI Used: Claude
//
// Input: Text file with a list of commands
// Output: Text listing email, date, and sender then showing how many emails are unread
//
// Created: 9/29/2026
// Revised: 9/30/2026 - 10/1/2026
// Revision: Handle incorrect file input, more comments, reduce copying

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <map>

/* -----------------------------------------------------------------------
 * class Email  [Author: GenAI (Claude)] [Revised: 10/1/2026 by Baylie Stith
 * (Made it easier to add/change categories)]
 * Represents one email and knows how to rank itself against another email.
 * Stores the raw sender category/subject/date, plus two derived values
 * (priorityRank_, dateValue_) computed once in the constructor so every
 * later comparison is a cheap integer comparison instead of string work.
 * isHigherPriorityThan() applies the two-step rule: compare category rank
 * first; only if categories match, fall back to comparing dates (newer
 * wins). display() prints the three required output lines for NEXT.
 * Input:  category/subject/date strings (constructor).
 * Output: none directly; display() prints to std::cout; other methods
 *         return the stored/derived values (getters, isHigherPriorityThan).
 * ----------------------------------------------------------------------- */
class Email {
public:
    Email(const std::string& senderCategory,
          const std::string& subject,
          const std::string& date)
        : senderCategory_(senderCategory),
          subject_(subject),
          date_(date),
          priorityRank_(categoryToRank(senderCategory)),
          dateValue_(parseDate(date))
    {
    }
 
    const std::string& getSenderCategory() const { return senderCategory_; }
    const std::string& getSubject()        const { return subject_; }
    const std::string& getDate()           const { return date_; }
    int  getPriorityRank() const { return priorityRank_; }
    long getDateValue()    const { return dateValue_; }
 
    // True if this email should be read before 'other': higher category
    // rank wins outright; a tie in category falls back to the newer date.
    bool isHigherPriorityThan(const Email& other) const {
        if (priorityRank_ != other.priorityRank_) {
            return priorityRank_ > other.priorityRank_;
        }
        return dateValue_ > other.dateValue_;
    }
 
    void display() const {
        std::cout << "Sender: "  << senderCategory_ << "\n";
        std::cout << "Subject: " << subject_         << "\n";
        std::cout << "Date: "    << date_            << "\n";
    }
 
private:
    std::string senderCategory_;
    std::string subject_;
    std::string date_;
    int  priorityRank_;   // bigger = more important sender category
    long dateValue_;      // bigger = more recent date
 
    /* Maps a sender-category string to an integer rank (Boss=5 highest
     * down to OtherPerson=1 lowest*/
    static int categoryToRank(const std::string& category) {
        static const std::map<std::string, int> priorities = {
            {"Boss", 5},
            {"Subordinate", 4},
            {"Peer", 3},
            {"ImportantPerson", 2},
            {"OtherPerson", 1}
        };

        auto it = priorities.find(category);

        if (it != priorities.end()) {
            return it->second;
        }

        return 0;
    }
 
    /* Converts "MM-DD-YYYY" into a single integer YYYYMMDD (e.g.
     * "03-01-2026" -> 20260301) so two dates can be compared with a
     * plain ">" and get the correct chronological answer. The stream
     * extraction below reads month, skips the '-', reads day, skips
     * the '-', then reads year. */
    static long parseDate(const std::string& date) {
        int month = 0, day = 0, year = 0;
        char dash1 = '\0', dash2 = '\0';
        std::istringstream iss(date);
        iss >> month >> dash1 >> day >> dash2 >> year;
        return static_cast<long>(year) * 10000L +
               static_cast<long>(month) * 100L +
               static_cast<long>(day);
    }
};
 
/* -----------------------------------------------------------------------
 * class MaxHeap  [Author: GenAI (Claude)] [Revised 10/1/2026 by Baylie Stith 
 * (reduced copying)]
 * A binary max-heap of Email objects stored in a std::vector (list-based,
 * no pointers/tree nodes). For an element at index i: parent = (i-1)/2,
 * left child = 2i+1, right child = 2i+2. The heap property (every parent
 * outranks its children, per Email::isHigherPriorityThan) guarantees the
 * single most important email always sits at index 0.
 *   insert()     - appends at the end, then heapifyUp to restore order.
 *   peekMax()    - O(1) look at index 0 without removing it.
 *   extractMax() - saves the root, moves the last element into its place,
 *                  shrinks the vector, then heapifyDown to restore order.
 *   heapifyUp/heapifyDown - the standard "bubble" repair steps, each
 *                  O(log n) because the heap is a complete binary tree.
 * Input:  an Email object (insert only); the other methods take no
 *         arguments and act on the heap's own internal data.
 * Output: peekMax/extractMax return the highest-priority Email; insert
 *         and the heapify helpers return nothing, they just reorder data_.
 * ----------------------------------------------------------------------- */
class MaxHeap {
public:
    MaxHeap() {}
 
    bool isEmpty() const { return data_.empty(); }
    int  size()    const { return static_cast<int>(data_.size()); }
 
    void insert(const Email& email) {
        data_.push_back(email);   // add as a new leaf at the end
        heapifyUp(size() - 1);    // bubble it up until order is restored
    }
 
    const Email& peekMax() const {
        if (isEmpty()) {
            throw std::runtime_error("peekMax() called on empty heap");
        }
        return data_[0];
    }
 
    void extractMax() {
        if (isEmpty()) {
            throw std::runtime_error("extractMax() called on empty heap");
        }
        data_[0] = data_[size() - 1];   // move last element into the root
        data_.pop_back();               // drop the now-duplicated last slot
        if (!isEmpty()) {
            heapifyDown(0);              // sift the new root down into place
        }
    }
 
private:
    std::vector<Email> data_;   // the whole heap, tree layout implied by index math
 
    int parentOf(int i)     const { return (i - 1) / 2; }
    int leftChildOf(int i)  const { return 2 * i + 1; }
    int rightChildOf(int i) const { return 2 * i + 2; }
 
    /* Repeatedly swaps the element at 'index' with its parent while it
     * outranks that parent, moving it toward the root; stops at the root
     * or once the parent already outranks it. */
    void heapifyUp(int index) {
        while (index > 0) {
            int p = parentOf(index);
            if (data_[index].isHigherPriorityThan(data_[p])) {
                std::swap(data_[index], data_[p]);
                index = p;
            } else {
                break;
            }
        }
    }
 
    /* Finds the highest-priority of {index, left child, right child}; if
     * that isn't 'index' itself, swaps it into place and repeats from the
     * child's old position, sinking the element down until both children
     * (if any) rank below it or it becomes a leaf. */
    void heapifyDown(int index) {
        int n = size();
        while (true) {
            int left = leftChildOf(index);
            int right = rightChildOf(index);
            int largest = index;
 
            if (left < n && data_[left].isHigherPriorityThan(data_[largest])) {
                largest = left;
            }
            if (right < n && data_[right].isHigherPriorityThan(data_[largest])) {
                largest = right;
            }
            if (largest == index) {
                break;
            }
            std::swap(data_[index], data_[largest]);
            index = largest;
        }
    }
};
 
/* -----------------------------------------------------------------------
 * class CEOInbox  [Author: GenAI (Claude)]
 * The object main() actually talks to. Owns a MaxHeap internally and
 * translates each command into the right heap operation, including the
 * empty-inbox and repeated-command edge cases:
 *   addEmail()    - builds an Email and inserts it (for EMAIL lines).
 *   showNext()    - peeks and prints the top email, or a message if empty;
 *                   does not remove anything (repeated NEXT shows the same email).
 *   readTopEmail()- removes the top email with no output; a no-op if empty
 *                   (repeated READ silently removes multiple emails).
 *   showCount()   - prints how many emails remain unread.
 * Input:  category/subject/date strings (addEmail only); the other three
 *         methods take no arguments.
 * Output: showNext() and showCount() print to std::cout; addEmail() and
 *         readTopEmail() return nothing, they just update the heap.
 * ----------------------------------------------------------------------- */
class CEOInbox {
public:
    void addEmail(const std::string& category,
                  const std::string& subject,
                  const std::string& date) {
        heap_.insert(Email(category, subject, date));
    }
 
    void showNext() const {
        if (heap_.isEmpty()) {
            std::cout << "No emails in inbox.\n";
            return;
        }
        heap_.peekMax().display();
    }
 
    void readTopEmail() {
        if (heap_.isEmpty()) {
            return;
        }
        heap_.extractMax();
    }
 
    void showCount() const {
        std::cout << "Unread emails: " << heap_.size() << "\n";
    }
 
private:
    MaxHeap heap_;
};
 
/* -----------------------------------------------------------------------
 * parseEmailFields()  [Author: GenAI (Claude)]
 * Splits the text after "EMAIL " (e.g. "Boss,Urgent review,03-01-2026")
 * into category/subject/date by locating the first two commas -- safe
 * because the subject itself is guaranteed comma-free. Returns false on
 * a malformed line (fewer than two commas) so it can be skipped.
 * Input:  rest (the text after "EMAIL "); category/subject/date are
 *         output parameters filled in by this function.
 * Output: returns true and fills category/subject/date on success, or
 *         returns false (leaving them untouched) if the line is malformed.
 * ----------------------------------------------------------------------- */
static bool parseEmailFields(const std::string& rest,
                              std::string& category,
                              std::string& subject,
                              std::string& date) {
    size_t firstComma = rest.find(',');
    if (firstComma == std::string::npos) return false;
    size_t secondComma = rest.find(',', firstComma + 1);
    if (secondComma == std::string::npos) return false;
 
    category = rest.substr(0, firstComma);
    subject  = rest.substr(firstComma + 1, secondComma - firstComma - 1);
    date     = rest.substr(secondComma + 1);
    return true;
}
 
/* -----------------------------------------------------------------------
 * processLine()  [Author: GenAI (Claude)]
 * Reads the first word of a line to identify the command, then dispatches
 * to the matching CEOInbox method. For EMAIL, getline() (not >>) is used
 * to grab the rest of the line so spaces in the subject are preserved;
 * the leading space left over after extracting the word "EMAIL" is then
 * trimmed off before parsing the comma-separated fields.
 * Input:  line (one raw line of text from the command file) and a
 *         reference to the CEOInbox to update.
 * Output: none returned; calls the matching CEOInbox method, which may
 *         print to std::cout (NEXT/COUNT) or just change inbox state.
 * ----------------------------------------------------------------------- */
static void processLine(const std::string& line, CEOInbox& inbox) {
    if (line.empty()) return;
 
    std::istringstream lineStream(line);
    std::string command;
    lineStream >> command;
 
    if (command == "EMAIL") {
        std::string rest;
        std::getline(lineStream, rest);
        if (!rest.empty() && rest[0] == ' ') {
            rest = rest.substr(1);
        }
        std::string category, subject, date;
        if (parseEmailFields(rest, category, subject, date)) {
            inbox.addEmail(category, subject, date);
        }
    } else if (command == "NEXT") {
        inbox.showNext();
    } else if (command == "READ") {
        inbox.readTopEmail();
    } else if (command == "COUNT") {
        inbox.showCount();
    }
    // Unrecognized/blank commands are ignored.
}
 
/* -----------------------------------------------------------------------
 * main()  [Author: GenAI (Claude)] [Revised: 10/1/2026 by Claude (Allowed
 * User to add file when running the program or input a file after running.
 * If the file doesn't exist it prompts user for new file unitl a usable
 * file is entered.)]
 * Reads the command file line by line and feeds each line to
 * processLine(), which builds up and queries a single CEOInbox. If a
 * filename was passed on the command line (argv[1]), that file is used
 * directly. Otherwise the program prompts the user to type a filename at
 * the console, and keeps asking again if the name they typed can't be
 * opened.
 * Input:  argc/argv (command-line arguments); argv[1], if present, is the
 *         command file's path. Otherwise a filename typed by the user in
 *         response to an on-screen prompt.
 * Output: prints the "enter a filename" prompt (and an error message if a
 *         typed filename can't be opened), plus the results of NEXT/COUNT
 *         commands as the file is processed. Returns 0 on success.
 * ----------------------------------------------------------------------- */
int main(int argc, char* argv[]) {
    CEOInbox inbox;
    std::string line;
    std::string filename;

    if (argc >= 2) {
        // Filename was already given on the command line; use it as-is.
        filename = argv[1];
    } else {
        // No filename given -- ask the user, and keep asking until a file
        // that actually opens is provided.
        std::ifstream testOpen;
        do {
            std::cout << "Enter the name of the command file: ";
            std::getline(std::cin, filename);
            testOpen.open(filename);
            if (!testOpen.is_open()) {
                std::cout << "Could not open file: " << filename << "\n";
            }
        } while (!testOpen.is_open());
        testOpen.close();
    }

    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Could not open file: " << filename << "\n";
        return 1;
    }
    while (std::getline(inFile, line)) {
        processLine(line, inbox);
    }

    return 0; // Exits Program