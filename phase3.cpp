#include <bits/stdc++.h>
using namespace std;

const int N = 32;
int r[N]; // array of registers

vector<bitset<32>> words;                                          // vector of instructions
map<long long int, long long int> memory;                          // map of memory
long long int im = 20000;                                          // size of memory
int k1, k2, k3, k4, k5;                                            // knobs
int cycle = 0;                                                     // stores the no of cycles
int pc = 0, next_pc = 0;                                           // it will store the next pc (in case of any jump);
int n_f = 0, n_d = 0, n_e = 0, n_m = 0, n_w = 0, n_a = 0, n_c = 0; // no of write back operations executed
int stall = 0, stalls = 0, stall_d = 0, stall_ds = 0;              // stall is due to control dependency
int ch = 0, dh = 0, bm = 0;                                        //  stall stores no of stall in a cycle stalls store total no of stalls
                                                                   // stall_d is stall due to data depenedency

// INSTRUCTIONS:
void FETCH();
void DECODE();
void EXECUTE();
void MEMORY_ACCESS();
void WRITE_BACK();

vector<bitset<32>> b_d;                              // pipelined register for fetch
vector<pair<bitset<32>, vector<long long int>>> b_e; // pipelined register for decode
vector<vector<long long int>> b_m, b_w;              // pipelined register for memory and write back

vector<pair<string, long long int>> b_f_w;
vector<vector<long long int>> b_d_w;
vector<vector<long long int>> b_e_w;
vector<vector<long long int>> b_m_w;
vector<vector<long long int>> b_w_w;

// for data forwarding
vector<int> result_d;
vector<int> r_d, rs_1, rs_2;               // for checking data dependency
vector<vector<int>> r_d_f;                 // stores value of destination registers to check stalls
string bin_hex(bitset<32> b);              // converts binary to hex
int bin_2_dec(bitset<32> b, int f, int l); // converts binary to decimal
// int control_pc;
map<int, pair<int, int>> btb; // branch target buffer : map(pc,target address, taken/not -taken)
                              // btb.first stores the instruction value in which branch is there
                              // btb.second checks if the branch is taken or not
                              // it is 1 if taken otherwise 0
                              // if taken then we will jump to the address which is stored in the pair

vector<int> p_c;
map<int, vector<int>> m_p;
int cwds = 0, pc_i;
ofstream fOut("terminal.txt");
ofstream FoUT("output.txt");

//----------- INTRODUCING CACHE -----------
int $_size, $_block_size, no_of_blocks, no_of_ways, no_of_sets; // no_of_sets determines the no of set in case of FA it is equal to 1
int tag_address, Index, block_offset;                           // a $block
int f_tag, set_no, f_bo;                                        // the location of the memory index we want to find
string assoc, rep_policy;                                       // assoc is associativity , rep_policy is the replacement policy

map<int, map<int, vector<int>>> mi$; // map of an instruction cache map.first represent set no
map<int, map<int, vector<int>>> md$; // map of an data cache map.first represent set no

void check_I_$(int next_pc); // it will check and update instruction cache
void check_D_$(int mem_loc); // it will check and update instruction cache

void replacement_I$(int set_no); // it will replace the block in a set in instruction cache
void replacement_D$(int set_no); // it will replace the block in a set in data cache

//----- DRIVER CODE -----

int main()
{
    //----------- Taking input for basic structure of a cache -----------
    cout << "Cache size (in KB): ";
    cin >> $_size;
    cout << "Cache block size (in KB): ";
    cin >> $_block_size;
    cout << "Direct mapped(DM)/Full Assoc (FA)/ Set Assoc(SA): ";
    cin >> assoc;
    if (assoc == "SA")
    {
        cout << "Number of ways for SA: ";
        cin >> no_of_ways;
    }
    else if (assoc == "FA")
        no_of_ways = no_of_blocks; // in case of fully associative , only one set is there
    else
        no_of_ways = 1; // in case of direct mapped , it is equal to no of blocks

    no_of_blocks = $_size / $_block_size;
    no_of_sets = no_of_blocks / no_of_ways;
    block_offset = log2($_block_size); //-------------------------------------in last check for Kilos it is in byte actually-------------------------------
    Index = log2(no_of_sets);
    tag_address = 32 - Index - block_offset;
    // checking for replacement policy
    if (assoc == "FA" || assoc == "SA")
    {
        cout << "Replacement policy: ";
        cin >> rep_policy;
    }

    /*printf("Print 1 to Enable a knob and O to disable a Knob: \n Knob 1: ");
    cin >> k1;
    if (k1)
    {
        cout << " Knob 2: ";
        cin >> k2;
        cout << " Knob 3: ";
        cin >> k3;
        cout << " Knob 4: ";
        cin >> k4;
        cout << " Knob 5: ";
        cin >> k5;
    }*/
    memory[0] = 0;
    while (im -= 4)
    {
        memory[im] = 0;
    }

    // UPDATING INITIAL VALUES OF x2 AND x3
    r[2] = 0X7FFFFFF0;
    r[3] = 0X10000000;

    // READING FROM THE FILE
    ifstream file("input.txt");
    if (!file.is_open())
    {
        cerr << "Error in opening the input file" << endl;
        return 1;
    }

    string line;
    while (getline(file, line))
    {

        stringstream ss(line);
        string word;
        string s;
        int j = 0;
        while (ss >> word)
        {

            if (j % 2)
            {
                if (sizeof(word) < 10)
                {
                    printf("Incorrect number of arguments. Please invoke the simulator !! \n");
                    break;
                }

                s = word;
                stringstream S;
                S << hex << s;
                unsigned n;
                S >> n;
                bitset<32> bit(n);
                words.push_back(bit);
            }

            j++;
        }
    }

    file.close();

    WRITE_BACK();

    // WRITING IN THE MEMORY FILE

    ofstream fout("Memory_file.txt");
    for (auto it : memory)
    {
        bitset<32> MEM(it.first);
        string s = (bin_hex(MEM));
        fout << s << "\t\t"; //
        bitset<32> MEm(it.second);
        s = bin_hex(MEm);
        fout << s[8] << s[9] << "\t" << s[6] << s[7] << "\t" << s[4] << s[5] << "\t" << s[2] << s[3] << endl;
    }
    fout.close();

    // WRITING IN THE REGISTER FILE IN HEXADECIMAL

    ofstream fot("register_file_hex.txt");
    int i = 0;
    for (auto it : r)
    {

        fot << "(x" << i << ")"
            << "\t"; //
        i++;
        bitset<32> MEm(it);
        string s = bin_hex(MEm);
        fot << s[2] << s[3] << s[4] << s[5] << s[6] << s[7] << s[8] << s[9] << endl;
    }
    fot.close();

    // WRITING IN THE REGISTER FILE IN DECIMAL

    ofstream FOUT("register_file_dec.txt");
    i = 0;
    for (auto it : r)
    {

        FOUT << "(x" << i << ")"
             << "\t"; //
        i++;

        FOUT << it << endl;
    }
    FOUT.close();
    FoUT << "• Stat1: Total number of cycles " << dec << cycle << endl;
    FoUT << "• Stat2: Total instructions executed " << dec << n_e << endl;
    FoUT << "• Stat3: CPI " << dec << cycle / n_e << endl;
    FoUT << "• Stat4: Number of Data-transfer (load and store) instructions executed " << dec << n_m << endl;
    FoUT << "• Stat5: Number of ALU instructions executed " << dec << n_a << endl;
    FoUT << "• Stat6: Number of Control instructions executed " << dec << n_c << endl;
    FoUT << "• Stat7: Number of stalls/bubbles in the pipeline " << dec << stall_ds + stalls << endl;
    FoUT << "• Stat8: Number of data hazards " << dec << dh << endl;
    FoUT << "• Stat9: Number of control hazards " << dec << ch << endl;
    FoUT << "• Stat10: Number of branch mispredictions " << dec << bm << endl;
    FoUT << "• Stat11: Number of stalls due to data hazards " << dec << stall_ds << endl;
    FoUT << "• Stat12: Number of stalls due to control hazards " << dec << stalls << endl;
    FoUT.close();
    return 0;
}

//------CHECKING AND UPDATING INSTRUCTION CACHE------
void check_I_$(int next_pc)
{
    bitset<32> i_p;
    bitset<32> find = next_pc;
    int f_tag = bin_2_dec(find, (31 - tag_address + 1), 31);
    int set_no = bin_2_dec(find, block_offset, (31 - tag_address));
    int f_bo = bin_2_dec(find, 0, (block_offset - 1));
    map<int, vector<int>> mpb;
    auto it = mi$.find(set_no);
    if (it != mi$.end()) /// initailly you have to make empty cache
        mpb = it->second;
    else
        cout << "Invalid input of cache datas!!!!!!!!!!!!";

    auto It = mpb.find(f_tag);
    if (It == mpb.end()) // miss (check replacement policies)
    {
        replacement_I$(set_no);
    }
    auto IT = mpb.find(f_tag);
    vector<int>v=IT->second;
    //bitset<4>in=v[]

    // cout << find << ' ' << f_tag << ' ' << f_I << ' ' << f_bo << "blockkkkkkkkkkkkkkkkk" << 31-tag_address+1 << endl;
}

//------CHECKING AND UPDATING DATA CACHE------
void check_D_$(int mem_loc)
{
    bitset<32> i_p;
    bitset<32> find = mem_loc;
    int f_tag = bin_2_dec(find, (31 - tag_address + 1), 31);
    int set_no = bin_2_dec(find, block_offset, (31 - tag_address));
    int f_bo = bin_2_dec(find, 0, (block_offset - 1));
    map<int, vector<int>> mpb;
    auto it = md$.find(set_no);
    if (it != md$.end()) /// initailly you have to make empty cache
        mpb = it->second;
    else
        cout << "Invalid input of cache datas!!!!!!!!!!!!";

    auto It = mpb.find(f_tag);
    if (It == mpb.end()) // miss (check replacement policies)
    {
        replacement_D$(set_no);
    }
    
}

//------REPLACEMENT OF A BLOCK IN INSTRUCTION CACHE------
void replacement_I$(int set_no)
{
}

//------REPLACEMENT OF A BLOCK IN INSTRUCTION CACHE------
void replacement_D$(int set_no)
{
}
//-----CONVERTING BINARY TO DECIMAL-----

int bin_2_dec(bitset<32> b, int f, int l)
{
    int mul = 1;
    int dec = 0;
    for (int i = f; i <= l; i++)
    {
        int x = b[i];
        dec = dec + x * mul;
        mul = mul << 1;
    }
    return dec;
}

//-----CONVERTING BINARY TO HEXADECIMAL-----

string bin_hex(bitset<32> b)
{
    string s = "0x00000000";
    int j = 2;
    for (int i = 31; i >= 0;)
    {
        int dec = (1 << 3) * b[i] + (1 << 2) * b[i - 1] + (1 << 1) * b[i - 2] + 1 * b[i - 3];
        if (dec < 10)
        {
            s[j] = 48 + dec;
        }
        else
        {
            s[j] = 97 + dec - 10;
        }
        i -= 4;
        j++;
    }
    return s;
}

//-----FETCH INSTRUCTION-----

void FETCH()
{
    cycle++;
    int x = words.size();
    int y = next_pc / 4;

    int i = 1;
    if (y < x)
    {
        check_I_$(next_pc);

        int no = next_pc;
        string s;
        int x = no / 16;
        b_f_w.push_back(make_pair(bin_hex(words[y]), (next_pc)));
        bitset<32> b = words[(y)];
        p_c.push_back(next_pc);
        i++;
        if (b_d.size())
            b_d.pop_back();
        b_d.push_back(b);
        auto it = btb.find(pc);
        if (it != btb.end() && (it->second.first))
        {
            pc_i = pc;
            pc = ((it->second).second) - 4;
            next_pc = pc;
        }
        n_f++;
    }

    if (cycle)
        fOut << "\n\nExecuting cycle no: " << dec << cycle << endl;
    if (b_f_w.size())
    {
        fOut << "FETCH:     Fetch instruction " << b_f_w[0].first << " from address 0x" << hex << b_f_w[0].second << "" << endl;
        b_f_w.pop_back();
    }
    if (b_d_w.size())
    {
        if (stall == 2)
            fOut << "DECODE:    STALL\n";
        else
        {
            switch (b_d_w[0][0])
            {
            case 1:
                fOut << "DECODE:    Operation is ADD, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 2:
                fOut << "DECODE:    Operation is SUB, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 3:
                fOut << "DECODE:    Operation is XOR, first operand R2, Second operand R3, destination register R1\n              Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 4:
                fOut << "DECODE:    Operation is OR, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 5:
                fOut << "DECODE:    Operation is AND, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 6:
                fOut << "DECODE:    Operation is SLL, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 7:
                fOut << "DECODE:    Operation is SRL, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 8:
                fOut << "DECODE:    Operation is SRA, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 9:
                fOut << "DECODE:    Operation is SLT, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 10:
                fOut << "DECODE:    Operation is SLTU, first operand R2, Second operand R3, destination register R1\n              Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << endl;

                break;

            case 18:
                fOut << "DECODE:    Operation is SLTI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 19:
                fOut << "DECODE:    Operation is SLTIU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 11:
                fOut << "DECODE:    Operation is ADDI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 12:
                fOut << "DECODE:    Operation is XORI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 13:
                fOut << "DECODE:    Operation is ORI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 14:
                fOut << "DECODE:    Operation is ANDI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 15:
                fOut << "DECODE:    Operation is SLLI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 16:
                fOut << "DECODE:    Operation is SRLI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 17:
                fOut << "DECODE:    Operation is SRAI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 20:
                fOut << "DECODE:    Operation is LB, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 21:
                fOut << "DECODE:    Operation is LH, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 22:
                fOut << "DECODE:    Operation is LW, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 23:
                fOut << "DECODE:    Operation is LBU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 24:
                fOut << "DECODE:    Operation is LHU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 25:
                fOut << "DECODE:    Operation is SB, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 26:
                fOut << "DECODE:    Operation is SH, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << b_d_w[0][1] << ", R3 = " << b_d_w[0][2] << ", imm = " << b_d_w[0][4] << endl;

                break;

            case 27:
                fOut << "DECODE:    Operation is SW, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 28:
                fOut << "DECODE:    Operation is BEQ, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 29:
                fOut << "DECODE:    Operation is BNE, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 30:
                fOut << "DECODE:    Operation is BLT, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 31:
                fOut << "DECODE:    Operation is BGE, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 32:
                fOut << "DECODE:    Operation is BLTU, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 33:
                fOut << "DECODE:    Operation is BGEU, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 34:
                fOut << "DECODE:    Operation is JAL, destination register R1, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 35:
                fOut << "DECODE:    Operation is JALR, destination register R1, first operand R2, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 36:
                fOut << "DECODE:    Operation is LUI, destination register R1, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 37:
                fOut << "DECODE:    Operation is AUIPC, destination register R1, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 38:
                fOut << "DECODE: Operation is ECALL " << endl;

                break;

            case 39:
                fOut << "DECODE: Operation is EBREAK " << endl;

                break;
            default:
                break;
            }
        }
        b_d_w.pop_back();

        // fOut << endl;
    }
    if (b_e_w.size())
    {
        if (stall == 2)
        {
            fOut << "EXECUTE:    STALL\n ";
        }
        else
        {
            switch (b_e_w[0][0])
            {
            case 1:
                fOut << "EXECUTE:     ADD "
                     << dec << r[b_e_w[0][2]] << " and " << dec << r[b_e_w[0][1]] << endl;

                break;

            case 2:
                fOut << "EXECUTE:    SUB "
                     << dec << r[b_e_w[0][2]] << " and " << dec << b_e_w[0][1] << endl;

                break;

            case 3:
                fOut << "EXECUTE:    XOR "
                     << dec << b_e_w[0][2] << " and " << dec << b_e_w[0][1] << endl;

                break;

            case 4:
                fOut << "EXECUTE:    Operation is OR, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][2] << " and " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 5:
                fOut << "EXECUTE:    Operation is AND, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 6:
                fOut << "EXECUTE:    Operation is SLL, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 7:
                fOut << "EXECUTE:    Operation is SRL, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 8:
                fOut << "EXECUTE:    Operation is SRA, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 9:
                fOut << "EXECUTE:    Operation is SLT, first operand R2, Second operand R3, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 10:
                fOut << "EXECUTE:    Operation is SLTU, first operand R2, Second operand R3, destination register R1\n              Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << endl;

                break;

            case 18:
                fOut << "EXECUTE:    Operation is SLTI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 19:
                fOut << "EXECUTE:    Operation is SLTIU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 11:
                fOut << "EXECUTE:    Operation is ADDI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 12:
                fOut << "EXECUTE:    Operation is XORI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 13:
                fOut << "EXECUTE:    Operation is ORI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 14:
                fOut << "EXECUTE:    Operation is ANDI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 15:
                fOut << "EXECUTE:    Operation is SLLI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 16:
                fOut << "EXECUTE:    Operation is SRLI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 17:
                fOut << "EXECUTE:    Operation is SRAI, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 20:
                fOut << "EXECUTE:    Operation is LB, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 21:
                fOut << "EXECUTE:    Operation is LH, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 22:
                fOut << "EXECUTE:    Operation is LW, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 23:
                fOut << "EXECUTE:    Operation is LBU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_e_w[0][1] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 24:
                fOut << "EXECUTE:    Operation is LHU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                     << dec << b_e_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 25:
                fOut << "EXECUTE:    Operation is SB, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 26:
                fOut << "EXECUTE:    Operation is SH, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << b_e_w[0][1] << ", R3 = " << b_e_w[0][2] << ", imm = " << b_e_w[0][4] << endl;

                break;

            case 27:
                fOut << "EXECUTE:    Operation is SW, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_e_w[0][1] << ", R3 = " << dec << b_e_w[0][2] << ", imm = " << dec << b_e_w[0][4] << endl;

                break;

            case 28:
                fOut << "EXECUTE:    Operation is BEQ, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_e_w[0][2] << ", R3 = " << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][5] << endl;

                break;

            case 29:
                fOut << "EXECUTE:    Operation is BNE, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 30:
                fOut << "EXECUTE:    Operation is BLT, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 31:
                fOut << "EXECUTE:    Operation is BGE, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_e_w[0][2] << ", R3 = " << dec << b_e_w[0][3] << ", imm = " << dec << b_e_w[0][5] << endl;

                break;

            case 32:
                fOut << "EXECUTE:    Operation is BLTU, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 33:
                fOut << "EXECUTE:    Operation is BGEU, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                     << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 34:
                fOut << "EXECUTE:    Operation is JAL, destination register R1, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 35:
                fOut << "EXECUTE:    Operation is JALR, destination register R1, first operand R2, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 36:
                fOut << "EXECUTE:    Operation is LUI, destination register R1, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 37:
                fOut << "EXECUTE:    Operation is AUIPC, destination register R1, immediate imm\n           Read registers R1 = "
                     << dec << b_d_w[0][3] << ", imm = " << dec << b_d_w[0][4] << endl;

                break;

            case 38:
                fOut << "EXECUTE: Operation is ECALL " << endl;

                break;

            case 39:
                fOut << "EXECUTE: Operation is EBREAK " << endl;

                break;
            default:
                break;
            }
            b_e_w.pop_back();
            // fOut << endl;
        }
    }
    if (b_m_w.size())
    {
        switch (b_m_w[0][0])
        {

        case 20:
            fOut << "MEMORY:    Operation is LB, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                 << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        case 21:
            fOut << "MEMORY:    Operation is LH, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                 << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        case 22:
            fOut << "MEMORY:    Operation is LW, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                 << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        case 23:
            fOut << "MEMORY:    Operation is LBU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                 << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        case 24:
            fOut << "MEMORY:    Operation is LHU, first operand R2, immediate imm, destination register R1\n           Read registers R1 = "
                 << dec << b_d_w[0][3] << ", R2 = " << dec << b_d_w[0][1] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        case 25:
            fOut << "MEMORY:    Operation is SB, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                 << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        case 26:
            fOut << "MEMORY:    Operation is SH, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                 << b_d_w[0][1] << ", R3 = " << b_d_w[0][2] << ", imm = " << b_d_w[0][4] << endl;

            break;

        case 27:
            fOut << "MEMORY:    Operation is SW, first operand R2, second operand R3, immediate imm\n           Read registers R2 = "
                 << dec << b_d_w[0][1] << ", R3 = " << dec << b_d_w[0][2] << ", imm = " << dec << b_d_w[0][4] << endl;

            break;

        default:
            fOut << "MEMORY:     No memory operation";
            break;
        }
        b_m_w.pop_back();
        fOut << endl;
    }
    // fOut << b_w_w.size();
    if (b_w_w.size())
    {
        // if (b_w_w[0][0] < 25 ||b_w_w[0][0] < 33)// && b_w_w[0][0] != 26 && b_w[0][0] != 27 && b_w[0][0] != 28 && b_w[0][0] != 29 && b_w[0][0] != 30 && b_w[0][0] != 31 && b_w[0][0] != 32 && b_w[0][0] != 33)
        if (b_w_w[0][2] != -1)
            fOut << "WRITEBACK: write " << b_w_w[0][1] << " to R" << b_w_w[0][2];
        else
            fOut << "No writeback  operation";

        b_w_w.pop_back();
        fOut << endl;
    }
    fOut << "stall_d= " << stall_d << endl;

    WRITE_BACK();
}

//-----DECODE INSTRUCTION-----

void DECODE()
{

    if (b_d.size())
    {
        int rs1, rs2, rd;
        long long imm;
        bitset<32> b = b_d[0];
        reverse(b_d.begin(), b_d.end());
        b_d.pop_back();
        if (b_e.size())
            b_e.pop_back();
        if (b[0] && b[1]) // might be a valid code
        {

            //-------Checking the opcode--------

            if (b[6] == 0 && b[5] == 1 && b[4] == 1 && b[3] == 0 && b[2] == 0)
            {
                // R type
                rs2 = bin_2_dec(b, 20, 24);
                rd = bin_2_dec(b, 7, 11);
                rs1 = bin_2_dec(b, 15, 19);
                //-------Checking func3------
                if (b[14] == 0 && b[13] == 0 && b[12] == 0)
                {
                    // add or sub

                    //-------Checking func7------
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // add
                        vector<long long int> val;
                        val.push_back(1);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val));
                    }
                    else if (b[31] == 0 && b[30] == 1 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // sub
                        vector<long long int> val;
                        val.push_back(2);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val));
                        // b_e.push_back(make_pair(b, 2));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
                {
                    // might be sll
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // sll
                        vector<long long int> val;
                        val.push_back(6);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val));
                        // b_e.push_back(make_pair(b, 6));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
                {
                    // might be slt
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // slt
                        vector<long long int> val;
                        val.push_back(9);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 9));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 0 && b[13] == 1 && b[12] == 1)
                {
                    // might be sltu
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // sltu
                        vector<long long int> val;
                        val.push_back(10);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 10));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
                {
                    // might be xor
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // xor
                        vector<long long int> val;
                        val.push_back(3);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 3));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
                {
                    // might be srl or sra
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // srl
                        vector<long long int> val;
                        val.push_back(7);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 7));
                    }
                    else if (b[31] == 0 && b[30] == 1 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // sra
                        vector<long long int> val;
                        val.push_back(8);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 8));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 1 && b[13] == 1 && b[12] == 0)
                {
                    // might be or
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // or
                        vector<long long int> val;
                        val.push_back(4);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 4));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 1 && b[13] == 1 && b[12] == 1)
                {
                    // might be and
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // and
                        vector<long long int> val;
                        val.push_back(5);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 5));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }
            }

            else if (b[6] == 0 && b[5] == 0 && b[4] == 1 && b[3] == 0 && b[2] == 0)
            {
                // I type
                // cout<<"I type";
                rs1 = bin_2_dec(b, 15, 19);
                rd = bin_2_dec(b, 7, 11);
                imm = bin_2_dec(b, 20, 31);
                if (b[31] == 1)
                    imm = -1 * (1 << 12) + imm;
                //-------Checking func3------
                if (b[14] == 0 && b[13] == 0 && b[12] == 0)
                {
                    // addi
                    rs2 = -23;
                    vector<long long int> val;
                    val.push_back(11);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 11));
                }

                else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
                {
                    // might be

                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        // slli
                        rs2 = -24;
                        vector<long long int> val;
                        val.push_back(15);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 15));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
                {
                    // might be slti
                    rs2 = -25;
                    vector<long long int> val;
                    val.push_back(18);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 18));
                }

                else if (b[14] == 0 && b[13] == 1 && b[12] == 1)
                {
                    // might be sltiu
                    rs2 = -26;
                    imm = bin_2_dec(b, 20, 31);
                    vector<long long int> val;
                    val.push_back(19);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 19));
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
                {
                    rs2 = -27;
                    // xori
                    vector<long long int> val;
                    val.push_back(12);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 12));
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
                {
                    // might be srli or srai
                    if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        rs2 = -28;
                        // srli
                        vector<long long int> val;
                        val.push_back(16);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 16));
                    }
                    else if (b[31] == 0 && b[30] == 1 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                    {
                        rs2 = -29;
                        // srai
                        vector<long long int> val;
                        val.push_back(17);
                        val.push_back(rs1);
                        val.push_back(rs2);
                        val.push_back(rd);
                        val.push_back(imm);
                        b_d_w.push_back(val);
                        b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 17));
                    }
                    else
                        cout << "Given instruction is invalid !!\n";
                }

                else if (b[14] == 1 && b[13] == 1 && b[12] == 0)
                {
                    // ori
                    rs2 = -30;
                    vector<long long int> val;
                    val.push_back(13);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 13));
                }

                else if (b[14] == 1 && b[13] == 1 && b[12] == 1)
                {
                    // andi
                    rs2 = -31;
                    vector<long long int> val;
                    val.push_back(14);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 14));
                }
            }

            else if (b[6] == 0 && b[5] == 0 && b[4] == 0 && b[3] == 0 && b[2] == 0)
            {
                // I type (load)

                rs1 = bin_2_dec(b, 15, 19);
                rd = bin_2_dec(b, 7, 11);
                imm = bin_2_dec(b, 20, 31);
                if (b[31] == 1)
                    imm = -1 * (1 << 12) + imm;

                //-------Checking func3------
                if (b[14] == 0 && b[13] == 0 && b[12] == 0)
                {
                    // lb
                    rs2 = -18;
                    vector<long long int> val;
                    val.push_back(20);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 20));
                }

                else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
                {
                    // lh
                    rs2 = -19;
                    vector<long long int> val;
                    val.push_back(21);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 21));
                }

                else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
                {
                    // lw
                    rs2 = -20;
                    vector<long long int> val;
                    val.push_back(22);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 22));
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
                {
                    rs2 = -21;
                    // lbu---Load byte(U)
                    imm = bin_2_dec(b, 20, 31);
                    vector<long long int> val;
                    val.push_back(23);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 23));
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
                {
                    rs2 = -22;
                    // lhu---Load half(U)
                    imm = bin_2_dec(b, 20, 31);
                    vector<long long int> val;
                    val.push_back(24);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 24));
                }
                else
                    cout << "Given instruction is invalid !!\n";
            }

            else if (b[6] == 0 && b[5] == 1 && b[4] == 0 && b[3] == 0 && b[2] == 0)
            {
                // S type

                rs1 = bin_2_dec(b, 15, 19);
                rs2 = bin_2_dec(b, 20, 24);
                long long int imm1 = bin_2_dec(b, 7, 11);
                long long int imm2 = bin_2_dec(b, 25, 31);
                imm = imm2 * (1 << 5) + imm1;
                if (b[31] == 1)
                    imm = -1 * (1 << 12) + imm;

                //-------Checking func3------
                if (b[14] == 0 && b[13] == 0 && b[12] == 0)
                {
                    // sb
                    rd = -15;
                    vector<long long int> val;
                    val.push_back(25);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 25));
                }

                else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
                {
                    // sh
                    rd = -16;
                    vector<long long int> val;
                    val.push_back(26);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 26));
                }

                else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
                {
                    // sw
                    rd = -17;
                    vector<long long int> val;
                    val.push_back(27);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 27));
                }
                else
                    cout << "Given instruction is invalid !!\n";
            }

            else if (b[6] == 1 && b[5] == 1 && b[4] == 0 && b[3] == 0 && b[2] == 0)
            {
                // B type

                rs1 = bin_2_dec(b, 15, 19);
                rs2 = bin_2_dec(b, 20, 24);
                int mul = 1;
                // rd = -1;
                imm = 0;
                for (int i = 8; i <= 11; i++)
                {
                    mul = mul << 1;
                    imm += b[i] * mul;
                }
                for (int i = 25; i <= 30; i++)
                {
                    mul = mul << 1;
                    imm += b[i] * mul;
                }
                mul = mul << 1;
                imm += b[7] * mul;
                mul = mul << 1;
                imm += b[31] * mul;
                if (b[31] == 1)
                    imm = -1 * (1 << 13) + imm;
                //-------Checking func3------
                if (b[14] == 0 && b[13] == 0 && b[12] == 0)
                {
                    // beq
                    rd = -14;
                    vector<long long int> val;
                    val.push_back(28);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 28));
                }

                else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
                {
                    // bne
                    rd = -13;
                    vector<long long int> val;
                    val.push_back(29);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 29));
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
                {
                    // blt
                    vector<long long int> val;
                    val.push_back(30);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 30));
                }

                else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
                {
                    // bge
                    rd = -11;
                    vector<long long int> val;
                    val.push_back(31);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 31));
                }

                else if (b[14] == 1 && b[13] == 1 && b[12] == 0)
                {
                    // bltu
                    rd = -10;
                    imm = (1 << 13) - imm;
                    vector<long long int> val;
                    val.push_back(32);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 32));
                }

                else if (b[14] == 1 && b[13] == 1 && b[12] == 1)
                {
                    // bgeu
                    rd = -1;
                    imm = (1 << 13) - imm;
                    vector<long long int> val;
                    val.push_back(33);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 33));
                }

                else
                    cout << "Given instruction is invalid !!\n";
            }

            else if (b[6] == 1 && b[5] == 1 && b[4] == 0 && b[3] == 1 && b[2] == 1)
            {
                // J type (jal)
                int mul = 1;
                int im = 0;
                for (int i = 21; i < 31; i++)
                {
                    mul = mul << 1;
                    im += b[i] * mul;
                }
                mul = mul << 1;
                im += b[20] * mul;
                for (int i = 12; i < 19; i++)
                {
                    mul = mul << 1;
                    im += b[i] * mul;
                }
                mul = mul << 1;
                im += b[31] * mul;
                if (b[31])
                    im = -1 * (1 << 20) + im;
                imm = im;
                rd = bin_2_dec(b, 7, 11);
                rs1 = -2;
                rs2 = -3;
                vector<long long int> val;
                val.push_back(34);
                val.push_back(rs1);
                val.push_back(rs2);
                val.push_back(rd);
                val.push_back(imm);
                b_d_w.push_back(val);
                b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 34));
            }

            else if (b[6] == 1 && b[5] == 1 && b[4] == 0 && b[3] == 0 && b[2] == 1)
            {
                // I type (jalr)

                rs1 = bin_2_dec(b, 15, 19);
                rs2 = -4;
                rd = bin_2_dec(b, 7, 11);
                long long imm = bin_2_dec(b, 20, 31);
                if (b[31])
                    imm = -1 * (1 << 12) + imm;
                vector<long long int> val;
                val.push_back(35);
                val.push_back(rs1);
                val.push_back(rs2);
                val.push_back(rd);
                val.push_back(imm);
                b_d_w.push_back(val);
                b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 35));
            }

            else if (b[6] == 0 && b[5] == 1 && b[4] == 1 && b[3] == 0 && b[2] == 1)
            {
                // U type (lui)

                rd = bin_2_dec(b, 7, 11);
                rs1 = -5;
                rs2 = -6;
                long long im = bin_2_dec(b, 12, 31);
                if (b[31])
                {
                    im = -1 * (1LL << 32) + im;
                }
                imm = im;
                vector<long long int> val;
                val.push_back(36);
                val.push_back(rs1);
                val.push_back(rs2);
                val.push_back(rd);
                val.push_back(imm);
                b_d_w.push_back(val);
                b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 36));
            }

            else if (b[6] == 0 && b[5] == 0 && b[4] == 1 && b[3] == 0 && b[2] == 1)
            {
                // U type (auipc)

                rd = bin_2_dec(b, 7, 11);
                rs1 = -7;
                rs2 = -8;
                long long im = bin_2_dec(b, 12, 31);

                if (b[31])
                {
                    im = -1 * (1LL << 32) + im;
                }
                imm = im;
                vector<long long int> val;
                val.push_back(37);
                val.push_back(rs1);
                val.push_back(rs2);
                val.push_back(rd);
                val.push_back(imm);
                b_d_w.push_back(val);
                b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 37));
            }

            else if (b[6] == 1 && b[5] == 1 && b[4] == 1 && b[3] == 0 && b[2] == 0)
            {
                // I type (ecall // ebreak)
                rs1 = bin_2_dec(b, 15, 19);
                rd = bin_2_dec(b, 7, 11);
                imm = bin_2_dec(b, 20, 31);
                if (imm == 0)
                {
                    // ecall
                    vector<long long int> val;
                    val.push_back(38);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_d_w.push_back(val);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 38));
                }
                else if (imm == 1)
                {
                    // ebreak
                    vector<long long int> val;
                    val.push_back(39);
                    val.push_back(rs1);
                    val.push_back(rs2);
                    val.push_back(rd);
                    val.push_back(imm);
                    b_e.push_back(make_pair(b, val)); // b_e.push_back(make_pair(b, 39));
                    b_d_w.push_back(val);
                }
                else
                    cout << "Given instruction is invalid !!\n";
            }

            else
                cout << "Given instruction is invalid !!\n";
        }
        else
            cout << "Given instruction is invalid !!\n";

        if (!cwds && pc == next_pc)
        {
            cout << rd << " =rd " << pc << " =pc " << next_pc << " =nextpc\n";
            vector<int> val;
            val.push_back(rd);
            val.push_back(rs1);
            val.push_back(rs2);
            auto itf = m_p.find(p_c[0]);
            if (stall == 0 && stall_d == 0 && pc == next_pc)
                m_p[p_c[0]] = val;
            p_c.pop_back();

            if (stall_d == 0) // ((itf == m_p.end()))
            {
                r_d.push_back(rd);
                rs_1.push_back(rs1);
                rs_2.push_back(rs2);
            }
            if (r_d.size() == 5)
            {
                auto it = r_d.begin();
                r_d.erase(it);
                it = rs_1.begin();
                rs_1.erase(it);
                it = rs_2.begin();
                rs_2.erase(it);
            }
            //----------checking stall due to data hazard----------
            if (r_d.size() == 4)
            {
                if ((rs_1[3] == r_d[2] || rs_2[3] == r_d[2]))
                    stall_d = 3;
                else if ((rs_1[2] != r_d[1]) && (rs_2[2] != r_d[1]) && ((rs_1[3] == r_d[1] || rs_2[3] == r_d[1])))
                    stall_d = 2;
                else if ((rs_1[1] != r_d[0]) && (rs_2[1] != r_d[0]) && (rs_1[2] != r_d[0]) && (rs_2[2] != r_d[0]) && (rs_1[2] != r_d[1]) && (rs_2[2] != r_d[1]) && (rs_1[3] == r_d[0] || rs_2[3] == r_d[0]))
                    stall_d = 1;
            }
            else if (r_d.size() == 3)
            {
                if ((rs_1[2] == r_d[1] || rs_2[2] == r_d[1]))
                    stall_d = 3;
                else if ((rs_1[1] != r_d[0]) && (rs_2[1] != r_d[0]) && ((rs_1[2] == r_d[0] || rs_2[2] == r_d[0])))
                    stall_d = 2;
            }
            else if ((rs_1[1] == r_d[0] || rs_2[1] == r_d[0]) && r_d.size() == 2)
                stall_d = 3;
            if (stall_d > 0)
                dh++;
            if (k2 && stall_d == 3)
                stall_ds += 1;
            else
                stall_ds += stall_d;
        }
    }
    if (stall_d != 0)
        cwds = 0;
    else
        cwds = 1;
    FETCH();
}

//-----EXECUTE INSTRUCTION-----

void EXECUTE()
{

    if (n_f && stall == 0 && stall_d == 0)
    {
        pc += 4;
        next_pc += 4;
    }
    else
        stall = 0;

    if (b_e.size()) // && stall == 0)
    {
        auto ittt = btb.find(pc_i);
        n_e++;
        bitset<32> b = b_e[0].first;
        int n = b_e[0].second[0];
        int rs1 = b_e[0].second[1];
        int rs2 = b_e[0].second[2];
        int rd = b_e[0].second[3];
        int imm = b_e[0].second[4];

        // reverse(b_e.begin(), b_e.end());
        int r_1, r_2;
        if (k2 && stall_d)
        {
            if ((rs_1[3] == r_d[0]) || (rs_1[3] == r_d[1]) || (rs_1[3] == r_d[2]) || (rs_1[1] == r_d[0]) || (rs_1[2] == r_d[0]) || (rs_1[2] == r_d[1]))
                r[rs1] = result_d[0];
            if ((rs_2[3] == r_d[0]) || (rs_2[3] == r_d[1]) || (rs_2[3] == r_d[2]) || (rs_2[1] == r_d[0]) || (rs_2[2] == r_d[0]) || (rs_2[2] == r_d[1]))
                r[rs2] = result_d[0];
            stall_d = 0;
            result_d.clear();
        }
        b_e.pop_back();
        long long x;
        if (n < 20 || n > 35)
            n_a++;
        else if (n > 27 && n < 36)
            n_c++;
        switch (n)
        {

        case 1: // add
            x = r[rs1] + r[rs2];
            cout << "EXECUTE:   ADD " << r[rs1] << " and " << r[rs2] << endl;
            break;

        case 2: // sub
            x = r[rs1] - r[rs2];
            cout << "EXECUTE:   SUBTRACT " << r[rs2] << " from " << r[rs1] << endl;
            break;

        case 3: // xor
            x = r[rs1] ^ r[rs2];
            cout << "EXECUTE:   XOR " << r[rs1] << " and " << r[rs2] << endl;
            break;

        case 4: // or
            x = r[rs1] | r[rs2];
            cout << "EXECUTE:   OR " << r[rs1] << " and " << r[rs2] << endl;
            break;

        case 5: // and
            x = r[rs1] & r[rs2];
            cout << "EXECUTE:   AND " << r[rs1] << " and " << r[rs2] << endl;
            break;

        case 6: // sll
            x = r[rs1] << r[rs2];
            cout << "EXECUTE:   LEFT SHIFT " << r[rs1] << " " << r[rs2] << " times" << endl;
            break;

        case 7: // srl
            x = abs(r[rs1] >> r[rs2]);
            cout << "EXECUTE:   LEFT SHIFT " << r[rs1] << " " << r[rs2] << " times" << endl;
            break;

        case 8: // sra
            x = r[rs1] >> r[rs2];
            cout << "EXECUTE:   RIGHT SHIFT ARITHMETIC " << r[rs1] << " " << r[rs2] << " times" << endl;
            break;

        case 9: // slt
            cout << "EXECUTE:   SHIFT LESS THAN " << r[rs1] << " and " << r[rs2] << endl;
            if (r[rs1] < r[rs2])
                x = 1;
            else
                x = 0;
            break;

        case 10: // sltu
            cout << "EXECUTE:   SHIFT LESS THAN UNSIGNED " << r[rs1] << " and " << r[rs2] << endl;

            if (abs(r[rs1]) < abs(r[rs2]))
                x = 1;
            else
                x = 0;
            break;

        case 11: // addi
            cout << "EXECUTE:   ADDI " << r[rs1] << " and " << imm << endl;
            x = r[rs1] + imm;
            break;

        case 12: // xori
            cout << "EXECUTE:   XORI " << r[rs1] << " and " << imm << endl;
            x = r[rs1] ^ imm;
            break;

        case 13: // ori
            cout << "EXECUTE:   ORI " << r[rs1] << " and " << imm << endl;
            x = r[rs1] | imm;
            break;

        case 14: // andi
            cout << "EXECUTE: ANDI " << r[rs1] << " and " << imm << endl;
            x = r[rs1] & imm;
            break;

        case 15: // slli
            cout << "EXECUTE:   LEFT SHIFT IMMEDIATE " << r[rs1] << " " << r[rs2] << " times" << endl;
            // cout << "Anisha anisha anisha Anisha anisha anisha"<<endl;
            x = r[rs1] << imm;
            break;

        case 16: // srli
            cout << "EXECUTE:   RIGHT SHIFT IMMEDIATE " << r[rs1] << " " << r[rs2] << " times" << endl;
            x = abs(r[rs1] >> imm);
            break;

        case 17: // srai
            cout << "EXECUTE:   RIGHT SHIFT ARITHEMTIC IMMEDIATE " << r[rs1] << " " << r[rs2] << " times" << endl;
            x = r[rs1] >> imm;
            break;

        case 18: // slti or sltiu
            cout << "EXECUTE:   SHIFT LESS THAN " << r[rs1] << " and " << r[rs2] << endl;
            if (r[rs1] < imm)
                x = 1;
            else
                x = 0;
            break;

        case 20: // lb
            cout << "EXECUTE:hello   LOAD BYTE M[" << r[rs1] << "+" << imm << "] [0:7] in " << r[rd] << endl;
            break;

        case 21: // lh
            cout << "EXECUTE:   LOAD BYTE M[" << r[rs1] << "+" << imm << "] [0:15] in " << r[rd] << endl;
            // cout<<
            break;

        case 22: // lw
            cout << "EXECUTE:   LOAD word M[" << r[rs1] << "+" << imm << "] [0:31] in " << r[rd] << endl;
            break;

        case 23: // lbu
            cout << "EXECUTE:   LOAD BYTE M[" << r[rs1] << "+" << imm << "] [0:7] in " << r[rd] << endl;
            break;

        case 24: // lhu
            cout << "EXECUTE:   LOAD BYTE M[" << r[rs1] << "+" << imm << "] [0:15] in " << r[rd] << endl;
            break;

        case 25: // sb
            cout << "EXECUTE:   STORE BYTE store " << r[rd] << " in M[" << r[rs1] << "+" << imm << "] [0:7]  " << endl;
            break;

        case 26: // sh
            cout << "EXECUTE:   STORE HALF store " << r[rd] << " in M[" << r[rs1] << "+" << imm << "] [0:15]  " << endl;
            break;

        case 27: // sw
            cout << "EXECUTE:   STORE WORD store " << r[rd] << " in M[" << r[rs1] << "+" << imm << "] [0:31]  " << endl;
            break;

        case 28: // beq
            if (r[rs1] == r[rs2])
            {
                ch++;
                if (ittt == btb.end() || (ittt->second).first == 0)
                    pc += imm - 4;
                btb[pc - 8] = make_pair(1, pc + imm - 4);
            }
            else
            {
                pc = next_pc;
                if (ittt != btb.end() && (ittt->second).first)
                {
                    bm++;
                    b_w.pop_back();
                    pc = ittt->first + 4;
                    b_d.pop_back();
                }
                btb[pc - 8] = make_pair(0, pc + imm - 4);
            }
            cout << "EXECUTE:   BRANCH if " << r[rs1] << "==" << r[rs2] << endl;
            break;

        case 29: // bne
            if (r[rs1] != r[rs2])
            {
                ch++;
                if (ittt == btb.end() || (ittt->second).first == 0)
                    pc += imm - 4;
                btb[pc - 8] = make_pair(1, pc + imm - 4);
            }
            else
            {
                pc = next_pc;
                if (ittt != btb.end() && (ittt->second).first)
                {
                    bm++;
                    b_w.pop_back();
                    pc = ittt->first + 4;
                    b_d.pop_back();
                }
                btb[pc - 8] = make_pair(0, pc + imm - 4);
            }
            cout << "EXECUTE:   BRANCH if " << r[rs1] << "!=" << r[rs2] << endl;
            break;

        case 30: // blt or bltu
            if (r[rs1] < r[rs2])
            {
                ch++;
                if (ittt == btb.end() || (ittt->second).first == 0)
                    pc += imm - 4;
                btb[pc - 8] = make_pair(1, pc + imm - 4);
            }
            else
            {
                bm++;
                cout << "xxxxxxxxxxxxxxxxxxxxxx" << endl;

                pc = next_pc;
                if (ittt != btb.end() && (ittt->second).first)
                {
                    cout << "xxxxxxxxxxxxxxxxxxxxxx" << endl;
                    b_d.pop_back();
                    pc = ittt->first + 4;
                    b_d.pop_back();
                }
                btb[pc - 8] = make_pair(0, pc + imm - 4);
            }
            cout << "EXECUTE:   BRANCH if " << r[rs1] << "<" << r[rs2] << endl;
            break;

        case 31: // bge or bgeu
            if (r[rs1] >= r[rs2])
            {
                ch++;
                if (ittt == btb.end() || (ittt->second).first == 0)
                    pc += imm - 4;
                btb[pc - 8] = make_pair(1, pc + imm - 4);
            }
            else
            {
                bm++;
                pc = next_pc;
                if (ittt != btb.end() && (ittt->second).first)
                {
                    b_w.pop_back();
                    pc = ittt->first + 4;
                    b_d.pop_back();
                }
                btb[pc - 8] = make_pair(0, pc + imm - 4);
            }
            cout << "EXECUTE:   BRANCH if " << r[rs1] << ">=" << r[rs2] << endl;
            break;
        case 34: // jal
        {
            ch++;
            if (ittt == btb.end()) // || (ii->second).first == 0)
                pc += imm - 4;
            btb[pc - 8] = make_pair(1, pc + imm - 4);
            x = pc - 4;
            cout << x << "=jal result\n";
            n = 34;
            cout << "EXECUTE:   JUMP AND LINK pc + " << imm << endl;
        }
        break;
        case 35: // jalr
            x = pc - 4;
            ch++;
            if (ittt == btb.end() || (ittt->second).first == 0)
                pc = r[rs1] + imm;
            btb[pc - 8] = make_pair(1, r[rs1] + imm - 4);
            cout << "EXECUTE:   JUMP AND LINK pc + " << r[rs1] << " + " << imm << endl;
            break;

        case 36: // lui
            x = (imm << 12);
            cout << "EXECUTE:   LOAD UPPER IMMEDIATE (" << imm << " << 12)" << endl;
            break;

        case 37: // auipc
            x = pc + (imm << 12) - 4;

            cout << "EXECUTE:   ADD UPPER IMMEDIATE TO PC  (PC + ( " << imm << " << 12))" << endl;
            break;

        case 38: // ecall
            x = r[rs1] || r[rs2];
            break;

        default:

            break;
        };
        vector<long long> val;
        val.push_back(n);
        val.push_back(x);
        val.push_back(rs1);
        val.push_back(rs2);
        val.push_back(rd);
        val.push_back(imm);
        b_e_w.push_back(val);
        if (b_m.size())
            b_m.pop_back();
        b_m.push_back(val);
        n_d++;
    }

    DECODE();
}

//-----MEMORY INSTRUCTION-----

void MEMORY_ACCESS()
{

    {
        int n;
        if (b_m.size())
        {
            n = b_m[0][0];
            long long x = b_m[0][1];
            int rs1 = b_m[0][2];
            int rs2 = b_m[0][3];
            int rd = b_m[0][4];
            int imm = b_m[0][5];
            // reverse(b_m.begin(), b_m.end());
            if (b_w.size())
                b_w.pop_back();
            b_m.pop_back();
            if ((n >= 1 && n < 20) || (n > 27))
            {
                cout << "MEMORY:    No memory  operation\n";
                if (k2)
                    result_d.push_back(x);
            }
            else
            {
                if (n > 24 && n < 28) // store
                {
                    n_m++;
                    bitset<32> B = r[rs2];
                    if (n == 27) // sw
                        memory[r[rs1] + imm] = r[rs2];
                    else if (n == 25) // sb
                    {

                        int y = bin_2_dec(B, 0, 7);
                        memory[r[rs1] + imm] = y;
                    }
                    else // sh
                    {
                        int y = bin_2_dec(B, 0, 15);
                        memory[r[rs1] + imm] = y;
                    }
                }
                else // load
                {
                    auto it = memory.find(r[rs1] + imm);
                    if (it == memory.end())
                    {
                        // cout << "The memory doesn't exist, can't perform load operation!\n";

                        /// b_w.push_back(make_pair(x, 25));
                        EXECUTE(); // 2 WRITE_BACK();
                    }
                    else
                    {
                        bitset<32> B = it->second;
                        if (n == 22)
                            x = it->second;
                        else if (n == 20 || n == 23)
                        {
                            x = bin_2_dec(B, 0, 7);
                        }
                        else
                            x = bin_2_dec(B, 0, 15);
                    }
                }
            }
            vector<long long int> val;
            val.push_back(n);
            val.push_back(x);
            val.push_back(rs1);
            val.push_back(rs2);
            val.push_back(rd);
            val.push_back(imm);
            b_m_w.push_back(val);
            b_w.push_back(val);
        }
        if (k2)
        {
            // if(stall_d==)
            if ((stall_d == 1) && n > 19 && n < 25 && result_d.size())
                EXECUTE();
            else if ((stall_d == 1 || stall_d == 2) && n < 20 && n > 24 && result_d.size())
                EXECUTE();
        }
        else if (stall_d != 0)
        {
            stall_d--;
            FETCH();
        }
        else
            EXECUTE();
    }
}

//-----WRITE BACK INSTRUCTION-----

void WRITE_BACK()
{
    cwds = 0;
    if (pc > 0 && b_d.size() == 0 && b_e.size() == 0 && b_m.size() == 0 && b_w.size() == 0)
    {
    }
    else
    {
        if (b_w.size())
        {
            int result = b_w[0][1];
            int n = b_w[0][0];
            int rd = b_w[0][4];
            vector<long long int> val;
            val.push_back(n);
            val.push_back(result);
            val.push_back(rd);
            b_w_w.push_back(val);
            b_w.pop_back();

            r[0] = 0;
            if (n > 24 && n < 34)
            {
            }
            else
            {
                r[rd] = result;
                if (n > 19 && n < 25 && k2)
                    result_d.push_back(result);
            }
            n_w++;
        }
        if (pc != next_pc)
        {
            next_pc = pc - 4;
            pc = next_pc;
            stall = 2;
            stalls += stall;
            r_d.push_back(-1);
            rs_1.push_back(-1);
            rs_2.push_back(-1);
            r_d.push_back(-1);
            rs_1.push_back(-1);
            rs_2.push_back(-1);
            while (r_d.size() >= 5)
            {
                auto it = r_d.begin();
                r_d.erase(it);
            }
            b_d.pop_back();
            b_e.pop_back();
        }
        MEMORY_ACCESS();
    }
    fOut.close();
}