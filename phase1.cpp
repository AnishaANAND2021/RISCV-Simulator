#include <bits/stdc++.h>
using namespace std;

const int N = 32;
int r[N];

vector<bitset<32>> words;
map<int, int> memory;

int rs1, rs2, rd, imm, pc = 0;
int next_pc = 0; // it will store the next pc (in case of any jump);

void FETCH();
void DECODE(bitset<32> b);
void EXECUTE(bitset<32> b, int n);
void MEMORY_ACCESS(int n, int x);
void WRITE_BACK(int result, int n);

string bin_hex(bitset<32> b);
int bin_2_dec(bitset<32> b, int f, int l);

int main()
{
    r[2] = 0X7FFFFFF0;
    r[3] = 0X0000000a;
    pc = 0; // it will store the next consecutive pc
    ifstream file("input.txt");
    if (!file.is_open())
    {
        cerr << "Error opening file" << endl;
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
    
    FETCH();

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
    cout<<"pc= "<<pc<<" next= "<<next_pc<<endl;
    return 0;
}

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

void FETCH()
{
    int x = words.size();
    int y = 0;
    int i = 1;
    while (y < x)
    {
        // cout << "pc=" << pc << "  np=" << next_pc << endl;
        if (pc != next_pc)
            next_pc = pc;
        y = next_pc / 4;
        bitset<32> b = words[(y)];
        pc += 4;
        next_pc += 4;
        i++;

        DECODE(b);
    }
}

void DECODE(bitset<32> b)
{
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
                    EXECUTE(b, 1);
                }
                else if (b[31] == 0 && b[30] == 1 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // sub
                    EXECUTE(b, 2);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
            {
                // might be sll
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // sll
                    EXECUTE(b, 6);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
            {
                // might be slt
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // slt
                    EXECUTE(b, 9);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 0 && b[13] == 1 && b[12] == 1)
            {
                // might be sltu
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // sltu
                    EXECUTE(b, 10);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
            {
                // might be xor
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // xor
                    EXECUTE(b, 3);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
            {
                // might be srl or sra
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // srl
                    EXECUTE(b, 7);
                }
                else if (b[31] == 0 && b[30] == 1 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // sra
                    EXECUTE(b, 8);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 1 && b[13] == 1 && b[12] == 0)
            {
                // might be or
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // or
                    EXECUTE(b, 4);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 1 && b[13] == 1 && b[12] == 1)
            {
                // might be and
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // and
                    EXECUTE(b, 5);
                }
                else
                    cout << "The given instruction is invalid !!\n";
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
                EXECUTE(b, 11);
            }

            else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
            {
                // might be slli
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // slli
                    EXECUTE(b, 15);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
            {
                // might be slti
                EXECUTE(b, 18);
            }

            else if (b[14] == 0 && b[13] == 1 && b[12] == 1)
            {
                // might be sltiu
                imm = bin_2_dec(b, 20, 31);
                EXECUTE(b, 18);
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
            {
                // xori
                EXECUTE(b, 12);
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
            {
                // might be srli or srai
                if (b[31] == 0 && b[30] == 0 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // srli
                    EXECUTE(b, 16);
                }
                else if (b[31] == 0 && b[30] == 1 && b[29] == 0 && b[28] == 0 && b[27] == 0 && b[26] == 0 && b[25] == 0)
                {
                    // srai
                    EXECUTE(b, 17);
                }
                else
                    cout << "The given instruction is invalid !!\n";
            }

            else if (b[14] == 1 && b[13] == 1 && b[12] == 0)
            {
                // ori
                EXECUTE(b, 13);
            }

            else if (b[14] == 1 && b[13] == 1 && b[12] == 1)
            {
                // andi
                EXECUTE(b, 14);
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
                EXECUTE(b, 20);
            }

            else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
            {
                // lh
                EXECUTE(b, 21);
            }

            else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
            {
                // lw
                EXECUTE(b, 22);
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
            {
                // lbu---Load byte(U)
                imm = bin_2_dec(b, 20, 31);
                EXECUTE(b, 23);
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
            {
                // lhu---Load half(U)
                imm = bin_2_dec(b, 20, 31);
                EXECUTE(b, 24);
            }
            else
                cout << "The given instruction is invalid !!\n";
        }

        else if (b[6] == 0 && b[5] == 1 && b[4] == 0 && b[3] == 0 && b[2] == 0)
        {
            // S type

            rs1 = bin_2_dec(b, 15, 19);
            rs2 = bin_2_dec(b, 20, 24);
            int imm1 = bin_2_dec(b, 7, 11);
            int imm2 = bin_2_dec(b, 25, 31);
            imm = imm2 * (1 << 5) + imm1;
            if (b[31] == 1)
                imm = -1 * (1 << 12) + imm;

            //-------Checking func3------
            if (b[14] == 0 && b[13] == 0 && b[12] == 0)
            {
                // sb
                EXECUTE(b, 25);
            }

            else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
            {
                // sh
                EXECUTE(b, 26);
            }

            else if (b[14] == 0 && b[13] == 1 && b[12] == 0)
            {
                // sw
                EXECUTE(b, 27);
            }
            else
                cout << "The given instruction is invalid !!\n";
        }

        else if (b[6] == 1 && b[5] == 1 && b[4] == 0 && b[3] == 0 && b[2] == 0)
        {
            // B type

            rs1 = bin_2_dec(b, 15, 19);
            rs2 = bin_2_dec(b, 20, 24);
            int mul = 1;
            imm = 0;
            // cout << "rs1= " << rs1 << " rs2= " << rs2 << endl;
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
                EXECUTE(b, 28);
            }

            else if (b[14] == 0 && b[13] == 0 && b[12] == 1)
            {
                // bne
                EXECUTE(b, 29);
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 0)
            {
                // blt
                EXECUTE(b, 30);
            }

            else if (b[14] == 1 && b[13] == 0 && b[12] == 1)
            {
                // bge
                EXECUTE(b, 31);
            }

            else if (b[14] == 1 && b[13] == 1 && b[12] == 0)
            {
                // bltu
                imm = (1 << 13) - imm;
                EXECUTE(b, 30);
            }

            else if (b[14] == 1 && b[13] == 1 && b[12] == 1)
            {
                // bgeu
                imm = (1 << 13) - imm;
                EXECUTE(b, 31);
            }

            else
                cout << "The given instruction is invalid !!\n";
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
            EXECUTE(b, 34);
        }

        else if (b[6] == 1 && b[5] == 1 && b[4] == 0 && b[3] == 0 && b[2] == 1)
        {
            // I type (jalr)

            rs1 = bin_2_dec(b, 15, 19);
            rd = bin_2_dec(b, 7, 11);
            imm = bin_2_dec(b, 20, 31);
            if (b[31])
                imm = -1 * (1 << 12) + imm;
            EXECUTE(b, 35);
        }

        else if (b[6] == 0 && b[5] == 1 && b[4] == 1 && b[3] == 0 && b[2] == 1)
        {
            // U type (lui)

            rd = bin_2_dec(b, 7, 11);
            long long im = (1 << 12) * bin_2_dec(b, 12, 31);
            if (b[31])
            {
                im = -1 * (1LL << 32) + im;
                imm = im;
            }
            EXECUTE(b, 36);
        }

        else if (b[6] == 0 && b[5] == 0 && b[4] == 1 && b[3] == 0 && b[2] == 1)
        {
            // U type (auipc)

            rd = bin_2_dec(b, 7, 11);
            long long im = (1 << 12) * bin_2_dec(b, 12, 31);

            if (b[31])
            {
                im = -1 * (1LL << 32) + im;
                imm = im;
            }
            EXECUTE(b, 37);
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
                EXECUTE(b, 38);
            }
            else if (imm == 1)
            {
                // ebreak
                EXECUTE(b, 39);
            }
            else
                cout << "The given instruction is invalid !!\n";
        }

        else
            cout << "The given instruction is invalid !!\n";
    }

    else
        cout << "The given instruction is invalid !!\n";
}

void EXECUTE(bitset<32> b, int n)
{

    int x;
    switch (n)
    {

    case 1: // add
        x = r[rs1] + r[rs2];
        break;

    case 2: // sub
        x = r[rs1] - r[rs2];
        break;

    case 3: // xor
        x = r[rs1] ^ r[rs2];
        break;

    case 4: // or
        x = r[rs1] | r[rs2];
        break;

    case 5: // and
        x = r[rs1] & r[rs2];
        break;

    case 6: // sll
        x = r[rs1] << r[rs2];
        break;

    case 7: // srl
        x = abs(r[rs1] >> r[rs2]);
        break;

    case 8: // sra
        x = r[rs1] >> r[rs2];
        break;

    case 9: // slt
        if (r[rs1] < r[rs2])
            x = 1;
        else
            x = 0;
        break;

    case 10: // sltu-----confusion------
        if (abs(r[rs1]) < abs(r[rs2]))
            x = 1;
        else
            x = 0;
        break;

    case 11: // addi

        x = r[rs1] + imm;
        break;

    case 12: // xori

        x = r[rs1] ^ imm;
        break;

    case 13: // ori

        x = r[rs1] | imm;
        break;

    case 14: // andi
        x = r[rs1] & imm;
        break;

    case 15: // slli
        x = r[rs1] << imm;
        break;

    case 16: // srli
        x = abs(r[rs1] >> imm);
        break;

    case 17: // srai
        x = r[rs1] >> imm;
        break;

    case 18: // slti or sltiu
        if (r[rs1] < imm)
            x = 1;
        else
            x = 0;
        break;

    case 20: // lb

        break;

    case 21: // lh

        break;

    case 22: // lw

        break;

    case 23: // lbu

        break;

    case 24: // lhu

        break;

    case 25: // sb

        break;

    case 26: // sh

        break;

    case 27: // sw

        break;

    case 28: // beq
        if (r[rs1] == r[rs2])
            pc += imm - 4;
        break;

    case 29: // bne
        if (r[rs1] != r[rs2])
            pc += imm - 4;
        break;

    case 30: // blt or bltu
        if (r[rs1] < r[rs2])
            pc += imm - 4;
        break;

    case 31: // bge or bgeu
        if (r[rs1] >= r[rs2])
            pc += imm - 4;
        // cout << "pc_bge=" << r[rs2] << endl;
        break;

    case 34: // jal
        x = pc;
        // cout<<"imm===="<<imm<<' '<<pc+imm-4<<endl;
        pc += imm - 4;
        cout << pc << endl;
        n = 34;
        break;
    case 35: // jalr
        r[rd] = pc;
        pc = r[rs1] + imm - 4;
        break;

    case 36: // lui
        r[rd] = (imm << 12);
        break;

    case 37: // auipc
        r[rd] = pc + (imm << 12) - 4;
        break;

    case 38: // ecall
        x = r[rs1] || r[rs2];
        break;

    default:

        break;
    };
    MEMORY_ACCESS(n, x);
}

void MEMORY_ACCESS(int n, int x)
{
    if ((n >= 1 && n < 20) || (n > 27))
    {
        // cout << "“MEMORY:No memory  operation”\n";
    }
    else
    {
        if (n > 24 && n < 28) // store
        {
            bitset<32> B = r[rs2];
            if (n == 27)
                memory[r[rs1] + imm] = r[rs2];
            else if (n == 26)
            {

                int y = bin_2_dec(B, 0, 7);
                memory[r[rs1] + imm] = y;
            }
            else
            {
                int y = bin_2_dec(B, 0, 15);
                memory[rs1 + imm] = y;
            }
        }
        else // load
        {
            auto it = memory.find(r[rs1] + imm);
            if (it == memory.end())
            {
                // cout << "The memory doesn't exist, can't perform load operation!\n";
                WRITE_BACK(x, 25);
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
    WRITE_BACK(x, n);
}

void WRITE_BACK(int result, int n)
{
    // cout << "“WRITE_BACK: ";
    r[0] = 0;
    if (n > 24 && n < 34)
    { // cout << "No writeback  operation”\n";
    }
    else
    {

        r[rd] = result;
    }
}