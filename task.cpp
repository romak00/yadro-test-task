#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <regex>
#include <queue>
#include <sstream>

class InputError {};

class ActionError {
    std::string error_time;
    std::string error_msg;
public:
    ActionError(const std::string& time, const std::string& msg) : error_time(time), error_msg(msg) {}
    std::string get_er_time() const { return error_time; }
    std::string get_er_msg() const { return error_msg; }
};

class Time {
public:

    int num;
    static inline int count;
    
    std::string time_string;

    std::pair<int, int> time_pair;

    int n;

    Time() : time_string("") {

        count++;
        num = count;

        std::cout << "Time constructor empty: " << num << " " << time_string + "  ts" <<'\n';
    }

    Time(const std::string& s_t) {
        count++;
        num = count;

        time_string = s_t;
        std::cout << "Time constructor string: " << num << " " << time_string + "  ts" << '\n';

        time_pair.first = std::stoi(s_t.substr(0, 2));
        time_pair.second = std::stoi(s_t.substr(3, 2));
    }

    Time& operator=(const Time& d_time) {
        time_string = d_time.time_string;
        time_pair = d_time.time_pair;
        return *this;
    }

    int time_in_minutes() const {
        return this->time_pair.first * 60 + this->time_pair.second;
    }

    bool operator<=(const Time& time) const {
        if (this->time_in_minutes() <= time.time_in_minutes()) {
            return true;
        }
        else {
            return false;
        }
    }

    bool operator>(const Time& time) const {
        if (*this <= time) {
            return false;
        }
        else {
            return true;
        }
    }

    ~Time() {
        std::cout << "Time destructor: " << num << " " << time_string + "  ts" << '\n';
    }

};

class Table {
public:
    bool is_occupied;
    std::string client_name;
    int time_sat_in_minutes;
    std::unique_ptr<Time> last_client_sat_time;
    int hours_rounded_up;

    Table() : is_occupied(false), client_name(""), time_sat_in_minutes(0), last_client_sat_time(nullptr), hours_rounded_up(0) {
        std::cout << "Table constructor: " << client_name << '\n';
    }

    void change_minutes_and_hours(const Time& time) {                                                     /// пересчет общего времени за столом, когда кто то ушел с него
        if (is_occupied) {                                                                                /// и округленных часов
            int client_sat_in_minutes = 0;
            if (*this->last_client_sat_time <= time) {
                client_sat_in_minutes = time.time_in_minutes() - this->last_client_sat_time->time_in_minutes();
                time_sat_in_minutes += client_sat_in_minutes;
                hours_rounded_up += client_sat_in_minutes / 60 + ((client_sat_in_minutes % 60 == 0) ? 0 : 1);
            }
            else {
                client_sat_in_minutes = 24 * 60 - this->last_client_sat_time->time_in_minutes() + time.time_in_minutes();
                time_sat_in_minutes += client_sat_in_minutes;
                hours_rounded_up += client_sat_in_minutes / 60 + ((client_sat_in_minutes % 60 == 0) ? 0 : 1);
            }
        }
    }

    void someone_left_the_table(const Time& time) {
        std::cout << " left the table    ";
        change_minutes_and_hours(time);                                                                   /// изменяем стол с которого ушли
        is_occupied = false;
        client_name = "";
        last_client_sat_time.reset();
    }

    void someone_sat_at_the_table(const std::string& time, const std::string client_who_sat) {
        std::cout << " sat table: ";
        is_occupied = true;                                                                             /// изменяем стол за который сели
        client_name = client_name;
        last_client_sat_time.reset();
        last_client_sat_time = std::make_unique<Time>(time);
    }


    ~Table() {
        std::cout << "Table destructor: " << client_name << '\n';
    }
};

class Computer_club {
public:
    const int price_per_hour;

    const std::shared_ptr<Time> opening_time;
    const std::shared_ptr<Time> closing_time;

    std::vector<Table> tables;

    std::map<std::string, std::pair<int, int>> client_list_with_tables;       // (0,0) - пришел; (n > 0,0) - в очереди, первое число - позиция 
                                                                              //                 (0,n > 0) - за столом, второе число - номер стола
    std::deque<std::string> client_queue;

public:

    Computer_club(const int number_of_tables, const std::shared_ptr<Time> o_t, const std::shared_ptr<Time> c_t, const int price) :
        tables(std::vector<Table>(number_of_tables)), opening_time(o_t), closing_time(c_t), price_per_hour(price) {
        
        std::cout << "Cc constructor: " << price_per_hour << '\n';
    }

    void remove_from_queue(const std::string& client_name) {
        auto iter = client_queue.begin() + (client_list_with_tables[client_name].first - 1);
        client_queue.erase(iter);
    }

    int empty_table_num() const{
        int ans = 0;
        for (int i = 0; i < tables.size(); i++) {
            if (!tables[i].is_occupied) {
                ans = i+1;
                break;
            }
        }
        return ans;
    }

    void action_1_client_came(const Time& time, const std::string& client_name) {
        if (client_list_with_tables.find(client_name) != client_list_with_tables.end()) {                  /// если уже в клубе
            throw ActionError(time.time_string, "YouShallNotPass");
        }
        else if ((*opening_time <= *closing_time && (*opening_time <= time && *closing_time > time)) ||    /// пришел вовремя
            (*opening_time > *closing_time && (*opening_time <= time || *closing_time > time))) {

            client_list_with_tables[client_name] = std::make_pair(0, 0);
        }
        else {                                                                                             /// пришел не вовремя
            throw ActionError(time.time_string, "NotOpenYet");
        }
    }
    
    void action_2_client_sat_at_the_table(const Time& time, const std::string& client_name, const int table_number) {
        if (client_list_with_tables.find(client_name) == client_list_with_tables.end()) {                  /// если такого нет
            throw ActionError(time.time_string, "ClientUnknown");
        }
        else if (tables[table_number-1].is_occupied) {                                                     /// если стол занят
            throw ActionError(time.time_string, "PlaceIsBusy");
        }
        else if (client_list_with_tables[client_name].second > 0) {                                        /// если пересаживается
            tables[client_list_with_tables[client_name].second - 1].someone_left_the_table(time.time_string);

            client_list_with_tables[client_name].second = table_number;

            tables[table_number - 1].someone_sat_at_the_table(time.time_string, client_name);
        }
        else {                                                                                             /// просто садится
            client_list_with_tables[client_name].second = table_number;

            tables[table_number - 1].someone_sat_at_the_table(time.time_string, client_name);
        }
    }

    void action_3_client_is_waiting(const Time& time, const std::string& client_name) {
        if (client_list_with_tables.find(client_name) == client_list_with_tables.end()) {                  /// если такого нет
            throw ActionError(time.time_string, "ClientUnknown");
        }
        else if (empty_table_num() > 0) {                                                              /// если есть свободный стол
            throw ActionError(time.time_string, "ICanWaitNoLonger!");
        }
        else {                                                                                             /// встает в очередь
            client_list_with_tables[client_name].first = client_queue.size() + 1;
            client_queue.push_back(client_name);
        }
    }

    void action_4_client_left(const Time& time, const std::string& client_name) {
        if (client_list_with_tables.find(client_name) == client_list_with_tables.end()) {                  /// если такого нет
            throw ActionError(time.time_string, "ClientUnknown");
        }
        else if (client_list_with_tables[client_name].first > 0) {                                         /// ушел когда находился в очереди
            remove_from_queue(client_name);
            client_list_with_tables.erase(client_name);
        }
        else if (client_list_with_tables[client_name].second == 0) {                                       /// просто ушел, когда не в очереди и не за столом
            client_list_with_tables.erase(client_name);
        }
        else {                                                                                             /// ушел когда за столом
            tables[client_list_with_tables[client_name].second - 1].someone_left_the_table(time.time_string);

            client_list_with_tables.erase(client_name);
        }
    }

    void action_11_client_left(const Time& time, const std::string& client_name) {
        if (client_list_with_tables[client_name].second > 0) {                                                     /// когда сидит за столом
            tables[client_list_with_tables[client_name].second - 1].someone_left_the_table(time.time_string);
        }
    }

    void action_12_client_sat_at_the_table(const Time& time, const std::string& client_name, const int table_number) {
        client_queue.pop_front();

        client_list_with_tables[client_name].second = table_number;
        client_list_with_tables[client_name].first = 0;

        tables[table_number - 1].someone_sat_at_the_table(time.time_string, client_name);
    }

    ~Computer_club() {
        std::cout << "Cc destructor: " << price_per_hour << '\n';
    }

};

class Check_input_format {

    std::string integer_regexp = "^[1-9][0-9]*$";
    std::string client_name_regexp = "^[a-z0-9_-]+$";
    std::string id_regexp = "^[1-4]$";
    std::string time_regexp = "^(0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]$";

public:
    void check_positive_int(const std::string& input) const {
        if (!std::regex_match(input, std::regex(integer_regexp))) {
            throw InputError();
        }
        else {
            try { int tmp = std::stoi(input); }
            catch (std::out_of_range) {
                throw InputError();
            }
        }
    }

    void check_time(const std::string& input) const {;
        if (!(std::regex_match(input, std::regex(time_regexp)))) {
            throw InputError();
        }
    }

    void check_id(const std::string& input) const {
        if (!(std::regex_match(input, std::regex(id_regexp)))) {
            throw InputError();
        }
    }

    void check_client_name(const std::string& input) const {
        if (!(std::regex_match(input, std::regex(client_name_regexp)))) {
            throw InputError();
        }
    }

    void check_table_num(const std::string & input, const int number_of_tables) const {
        check_positive_int(input);
        if (std::stoi(input) > number_of_tables) {
            throw InputError();
        }
    }

    void check_tables_total_num_and_price(const std::string& input) const {
        std::istringstream iss(input);
        std::string inp_num = "";
        iss >> inp_num >> std::ws;
        if (!iss.eof()) {
            throw InputError();
        }
        else {
            check_positive_int(inp_num);
        }
    }

    void check_op_cl_time(const std::string& input) const {
        std::istringstream iss(input);
        std::string op_t = "", cl_t = "";
        iss >> op_t >> cl_t >> std::ws;
        if (!iss.eof()) {
            throw InputError();
        }
        else {
            check_time(op_t);
            check_time(cl_t);
        }
    }

    void check_action(const std::string& input, const int number_of_tables) const {
        std::istringstream iss(input);
        std::string time = "", id = "", cl_name = " ", table = " ";
        iss >> time >> id >> cl_name >> std::ws;
        check_time(time);
        check_id(id);
        check_client_name(cl_name);
        if (std::stoi(id) == 2 && iss.eof()) {
            throw InputError();
        }
        else if (std::stoi(id) == 2 && !iss.eof()) {
            iss >> table >> std::ws;
            check_table_num(table, number_of_tables);
            if (!iss.eof()) {
                throw InputError();
            }
        }
        else if (std::stoi(id) != 2 && !iss.eof()){
            throw InputError();
        }
    }

    void check_time_order(const std::shared_ptr<Time> opening_time, const std::shared_ptr<Time> closing_time, const Time& prev_time, const Time& curr_time, const int id) {
        static bool was_24h_crossed = false;
        static bool ot_less_than_ct = opening_time <= closing_time;

        if (id != 1 && ot_less_than_ct && (*opening_time > curr_time || *closing_time <= curr_time)) {              // если действие 1, то оно может быть невовремя
            throw InputError();                                                                                   // a если не 1, то должно быть вовремя 
        }
        else if (id != 1 && !ot_less_than_ct && *opening_time > curr_time && *closing_time <= curr_time) {
            throw InputError();
        }

        if (ot_less_than_ct && prev_time.time_string != "") {
            if (prev_time > curr_time) {
                throw InputError();
            }
        }
        else if (!ot_less_than_ct && prev_time.time_string == "") {
            was_24h_crossed = *opening_time > curr_time;
        }
        else if (!ot_less_than_ct && prev_time.time_string != "") {
            if (was_24h_crossed) {
                if (prev_time > curr_time || *opening_time <= curr_time) {
                    throw InputError();
                }
            }
            else {
                if (prev_time > curr_time) {
                    if (curr_time > *opening_time) {
                        throw InputError();
                    }
                    else {
                        was_24h_crossed = true;
                    }
                }
            }
        }
    }

};

int main(int argc, char** argv) {
    std::string sourse_file_name = argv[1];
    std::ifstream fin;
    fin.open(sourse_file_name);                               // в файле конец строки должен быть LF (т. е. \n), по крайней мере у меня иначе getline не работает

    int number_of_tables = 0;
    std::string opening_time_str= "", closing_time_str= "";
    int price_per_hour = 0;

    std::string tmp_str = "";

    std::stringstream main_output, secondary_output;

    Check_input_format checker;

    std::getline(fin, tmp_str);
    try {
        checker.check_tables_total_num_and_price(tmp_str);
        number_of_tables = std::stoi(tmp_str);
    }
    catch (const InputError& er) {
        std::cerr << tmp_str;
        fin.close();
        return 1;
    }

    std::getline(fin, tmp_str);
    try {
        checker.check_op_cl_time(tmp_str);
        std::istringstream iss(tmp_str);
        iss >> opening_time_str >> closing_time_str;
        main_output << opening_time_str << '\n';
    }
    catch (const InputError& er) {
        std::cerr << tmp_str;
        fin.close();
        return 1;
    }

    std::getline(fin, tmp_str);
    try {
        checker.check_tables_total_num_and_price(tmp_str);
        price_per_hour = std::stoi(tmp_str);
    }
    catch (const InputError& er) {
        std::cerr << tmp_str;
        fin.close();
        return 0;
    }

    std::shared_ptr<Time> opening_time = std::make_shared<Time>(opening_time_str);
    std::shared_ptr<Time> closing_time = std::make_shared<Time>(closing_time_str);

    Computer_club club(number_of_tables, opening_time, closing_time, price_per_hour);

    std::string time = "";
    int id = 0;
    std::string client_name = "";
    int table_number;

    Time prev_time;

    while (std::getline(fin, tmp_str)) {
        std::cout << "end of curr time" << "+++++++++++++" << '\n';
        try {
            checker.check_action(tmp_str, number_of_tables);

            std::istringstream iss(tmp_str);
            iss >> time >> id >> client_name;

            std::cout << "create curr time -------" << '\n';
            Time curr_time(time);
            std::cout << "create curr time -------" << '\n';

            checker.check_time_order(opening_time, closing_time, prev_time, curr_time, id);

            main_output << tmp_str << '\n';
            try {
                switch (id) {
                case 1:
                    club.action_1_client_came(curr_time, client_name);
                    break;
                case 2:
                    iss >> table_number;
                    club.action_2_client_sat_at_the_table(curr_time, client_name, table_number);
                    break;
                case 3:
                    club.action_3_client_is_waiting(curr_time, client_name);
                    if (club.client_queue.size() > club.tables.size()) {
                        club.remove_from_queue(client_name);
                        club.client_list_with_tables.erase(client_name);
                        main_output << curr_time.time_string << " 11 " << client_name << '\n';
                    }
                    break;
                case 4:                                                             
                    bool no_empty_tables = club.empty_table_num() == 0;
                    club.action_4_client_left(curr_time, client_name);
                    int empty_table = club.empty_table_num();
                    if (no_empty_tables && (empty_table > 0) && !club.client_queue.empty()) {
                        std::string first_in_queue = club.client_queue.front();
                        club.action_12_client_sat_at_the_table(curr_time, club.client_queue.front(), empty_table);
                        main_output << curr_time.time_string << " 12 " + first_in_queue << " " << empty_table << '\n';
                    }
                    break;
                }
            }
            catch (const ActionError& er) {
                if (er.get_er_msg() == "NotOpenYet" && prev_time.time_string != "") {
                    if ((*opening_time <= *closing_time && (*opening_time <= prev_time && *closing_time > prev_time)) ||
                        (*opening_time > *closing_time && (*opening_time <= prev_time || *closing_time > prev_time))) {
                        
                        secondary_output << er.get_er_time() << " 13 " << er.get_er_msg() << '\n';
                    }
                }
                main_output << er.get_er_time() << " 13 " << er.get_er_msg() << '\n';
            }
            prev_time = curr_time;
            std::cout << "end of curr time" << "+++++++++++++" << '\n';
        }
        catch (const InputError& er) {
            std::cerr << tmp_str << '\n';
            fin.close();
            return 1;
        }
    }

    std::string cl_name_lft = "";

    if (!club.client_list_with_tables.empty()) {
        for (std::map<std::string, std::pair<int, int>>::iterator i = club.client_list_with_tables.begin(); i != club.client_list_with_tables.end(); i++) {
            cl_name_lft = i->first;
            main_output << closing_time->time_string << " 11 " << cl_name_lft << '\n';
            club.action_11_client_left(*closing_time, cl_name_lft);
        }
    }

    main_output << secondary_output.rdbuf();

    main_output << closing_time_str << '\n';
    std::cout << main_output.rdbuf();

    int i = 0;
    for (auto& table : club.tables) {
        std::cout << ++i << " " << price_per_hour * table.hours_rounded_up << " " << table.time_sat_in_minutes << '\n';
    }
    

    

    fin.close();
    return 0;
}