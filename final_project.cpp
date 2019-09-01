#include<fstream>
#include<string>
#include<vector>
#include<cstdio>
using namespace std;
enum R_type_mapping {ADD, SUB, AND, OR, SLT};
enum I_type_mapping {ANDI, LW, SW, ADDI, BEQ};
enum R_I_type_def {R, I};
string R_type_function[] = {"100000", "100010", "100100", "100101", "101010"};
string I_type_op[] = {"001100", "100011", "101011", "001000", "000100"};
vector<string> input_instruction;
class reg
{
public:
    int reg_location;
    int store_value;
    int lw_memory;
};
class memory
{
public:
    int memory_location;
    int store_value;
};
class instruction_def
{
public:
    int type;
    int cycle;
    int rs, rt, rd;
    int rs_value, rt_value;
    string instruction;
    string control_sign;
    int op;
    int fun;
    int immediate;
    int beq_sol;
    class memory store_mem;
    class reg store_reg;
};

int binary_to_decimal(string a)
{
    int ans = 0;
    int tmp = 1;
    for(int i = a.size() - 1; i >= 0 ; i--)
    {
        ans = ans + (a[i] - '0') * tmp;
        tmp = tmp * 2;
    }
    return ans;
}

int main()
{

    /*input file name and output file name*/
    string input_file_name[4] = {"General", "Datahazard", "lwhazard", "Branchhazard"};
    string output_file_name[4] = {"genResult.txt", "dataResult.txt", "loadResult.txt", "branchResult.txt"};
    for(int i = 0; i < 4; i++)
    {
        /*memory register pc*/
        int cur_register[10] = {0, 9, 5, 7, 1, 2, 3, 4, 5, 6};
        int me[5] = {5, 9, 4, 8, 7};
        int pc = 0;
        string input = "", inputstr;

        /*find the output file, if not, create new file*/
        fstream ut;
        ut.open(output_file_name[i].c_str());
        if(ut.fail())
            ofstream fout(output_file_name[i].c_str());
        ut.close();

        /*output file*/
        fstream cout;
        cout.open(output_file_name[i].c_str(), fstream::out | fstream::trunc);

        /*input file*/
        ifstream in(input_file_name[i].c_str(), fstream::in);
        while(getline(in, inputstr))
            input = input + inputstr;

        int len = input.size() - 1;
        int d = 0;

        /*each instruction has 32 bits, and input can be used to find the instruction.*/
        while(len > 0)
        {
            string store = input.substr(d, 32);
            input_instruction.push_back(store);
            len = len - 32;
            d = d + 32;
        }

        /*store data while executing*/
        vector<class instruction_def> store_instruction;
        vector<class instruction_def> id_ex, ex_mem, mem_wb;
        vector<string> if_id;
        vector<int>control(4, 0);
        int cycle = 1;

        /*flag of lw_hazard branch */
        bool lw_hazard = false;
        bool branch = false;
        bool cancel_branch = false;

        /*start*/
        while(!(input_instruction.size()==0 && if_id.size()==0 && id_ex.size()==0 && ex_mem.size()==0 && mem_wb.size() == 0))
        {
            /*print register and memory*/
            cout << "CC " << cycle << ":" << endl;
            cout << endl;
            cout << "Register" << endl;
            for(int i = 0; i < 10; i++)
                cout << "$" << i << ": " << cur_register[i] << endl;
            cout << endl;
            cout << "Data memory:" << endl;
            cout << "0x00:" << me[0]  << endl;
            cout << "0x04:" << me[1]  << endl;
            cout << "0x08:" << me[2]  << endl;
            cout << "0x0C:" << me[3]  << endl;
            cout << "0x10:" << me[4]  << endl;
            cout << endl;

            /*control can determine which cycle must execute*/
            control.insert(control.begin(), 1);

            /*lw_hazard*/
            if(lw_hazard == true)
                control[2] = 0;

            /*branch*/
            if(branch == true)
            {
                control[1] = 0;
                if_id.erase(if_id.begin());
                id_ex.erase(id_ex.begin());
                cancel_branch = true;
            }
            if(cancel_branch == true)
            {
                cancel_branch = false;
                branch = false;
            }

            /* IF */
            {
                string tm = input_instruction[0];
                if(lw_hazard == true)
                {
                    cout << "IF/ID:" << endl;
                    cout << "PC		" << pc << endl;
                    cout << "Instruction	" << tm << endl;
                }
                else
                    pc = pc + 4;

                if(input_instruction.size() != 0 && control[0] == 1 && lw_hazard == false)
                {
                    if_id.push_back(tm);
                    input_instruction.erase(input_instruction.begin());
                    cout << "IF/ID:" << endl;
                    cout << "PC		" << pc << endl;
                    cout << "Instruction	" << tm << endl;
                }
                else if(lw_hazard == false)
                {
                    cout << "IF/ID:" << endl;
                    cout << "PC		" << pc << endl;
                    cout << "Instruction	00000000000000000000000000000000"  << endl;
                }
            }

            cout << endl;

            /*ID*/
            {
                if(cycle > 1 && if_id.size() != 0 && control[1] == 1)
                {
                    string t = if_id[0];
                    class instruction_def *tmp = new class instruction_def();


                    for(int i = 0; i < t.size(); i++)
                    {
                        string op = t.substr(0, 6);
                        if(op == "000000")
                        {
                            /*get instruction*/
                            string fun = t.substr(26, 6);
                            string rs = t.substr(6, 5);
                            string rt = t.substr(11, 5);
                            string rd = t.substr(16, 5);
                            string immediate = t.substr(16, 16);
                            tmp->type = R;
                            tmp->instruction = t;
                            tmp->op = 0;
                            tmp->rs = binary_to_decimal(rs);
                            tmp->rt = binary_to_decimal(rt);
                            tmp->rs_value = cur_register[tmp->rs];
                            tmp->rt_value = cur_register[tmp->rt];
                            tmp->rd = binary_to_decimal(rd);
                            tmp->immediate = binary_to_decimal(immediate);
                            tmp->cycle = cycle;
                            for(int j = 0; j < 5; j++)
                            {
                                if(fun == R_type_function[j])
                                {
                                    tmp->fun = j;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            string fun = t.substr(26, 6);
                            string rs = t.substr(6, 5);
                            string rt = t.substr(11, 5);
                            string rd = t.substr(16, 5);
                            string immediate = t.substr(16, 16);
                            for(int j = 0; j < 5; j++)
                            {
                                if(op == I_type_op[j])
                                {
                                    tmp->op = j;
                                    break;
                                }
                            }
                            tmp->type = I;
                            tmp->instruction = t;
                            tmp->rs = binary_to_decimal(rs);
                            tmp->rt = binary_to_decimal(rt);
                            tmp->rs_value = cur_register[tmp->rs];
                            tmp->rt_value = cur_register[tmp->rt];
                            tmp->rd = binary_to_decimal(rd);
                            tmp->immediate = binary_to_decimal(immediate);
                            tmp->cycle = cycle;
                        }

                    }

                    if(lw_hazard == false)
                    {
                        if_id.erase(if_id.begin());
                        id_ex.push_back(*tmp);
                    }

                    /*print id/ex*/
                    cout << "ID/EX :" << endl;
                    cout << "ReadData1	" << tmp->rs_value << endl;
                    cout << "ReadData2	" << tmp->rt_value << endl;
                    cout << "sign_ext	" << tmp->immediate << endl;
                    cout << "Rs		" << tmp -> rs << endl;
                    cout << "Rt		" << tmp -> rt << endl;
                    cout << "Rd		" << tmp -> rd << endl;
                    cout << "Control signals	";
                    if(tmp -> type == R)
                        cout << "110000010" << endl;
                    else if(tmp -> type == I)
                    {
                        if(tmp->op == LW)
                            cout << "000101011" <<endl;
                        else if(tmp->op == SW)
                            cout << "000100100" << endl;
                        else if(tmp->op == BEQ)
                            cout << "001010000" << endl;
                        else if(tmp->op==ADDI)
                            cout << "000100010" << endl;
                        else if(tmp->op == ANDI)
                            cout << "011100010" << endl;
                    }
                }
                else
                {
                    cout << "ID/EX :" << endl;
                    cout << "ReadData1	0" << endl;
                    cout << "ReadData2	0" << endl;
                    cout << "sign_ext	0" << endl;
                    cout << "Rs		0" << endl;
                    cout << "Rt		0" << endl;
                    cout << "Rd		0" << endl;
                    cout << "Control signals	000000000" << endl;
                }
            }

            cout << endl;

            /*exe*/
            if(cycle > 2 && id_ex.size() != 0 && lw_hazard == false && control[2] == 1)
            {
                class instruction_def *tmp = new class instruction_def();
                *tmp = id_ex[0];
                if(tmp->type == R)
                {
                    if(tmp->fun == ADD)
                    {
                        int sol = tmp->rs_value + tmp->rt_value;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rd;
                    }
                    else if(tmp->fun == SUB)
                    {
                        int sol = tmp->rs_value - tmp->rt_value;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rd;
                    }
                    else if(tmp->fun == AND)
                    {
                        int sol = tmp->rs_value & tmp->rt_value;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rd;
                    }
                    else if(tmp->fun == OR)
                    {
                        int sol = tmp->rs_value | tmp->rt_value;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rd;
                    }
                    else if(tmp->fun == SLT)
                    {
                        int sol = 0;
                        if(tmp->rs_value < tmp->rt_value)
                            sol = 1;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rd;
                    }
                }
                else if(tmp -> type == I)
                {
                    if(tmp -> op == ANDI)
                    {
                        int sol = tmp->rs_value & tmp->immediate;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rt;
                    }
                    else if(tmp->op == LW)
                    {
                        int sol = (tmp->immediate+tmp->rs_value);
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rt;
                    }
                    else if(tmp->op == SW)
                    {
                        int sol = (tmp->immediate+tmp->rs_value);
                        (tmp->store_mem).store_value = tmp->rt;
                        (tmp->store_mem).memory_location = sol;
                    }
                    else if(tmp->op == ADDI)
                    {
                        int sol = tmp->rs_value + tmp->immediate;
                        (tmp->store_reg).store_value = sol;
                        (tmp->store_reg).reg_location = tmp->rt;
                    }
                    else if(tmp -> op == BEQ)
                    {
                        if(tmp->rs_value == tmp->rt_value)
                        {
                            pc = pc +  4 * (tmp->immediate - 2) ;
                            for(int k = 0; k < (tmp->immediate) - 2; k++)
                            {
                                input_instruction.erase(input_instruction.begin());
                            }
                            branch = true;
                        }

                        tmp->beq_sol = tmp->rs_value - tmp->rt_value;
                    }
                }
                id_ex.erase(id_ex.begin());
                ex_mem.push_back(*tmp);


                cout << "EX/MEM :" << endl;
                if(tmp->type == R)
                {
                    cout << "ALUout		" << (tmp->store_reg).store_value << endl;
                }
                else if(tmp->type == I)
                {
                    if(tmp->op == SW)
                        cout << "ALUout		" << (tmp->store_mem).memory_location << endl;
                    else if(tmp->op == BEQ)
                        cout << "ALUout		" << tmp->beq_sol << endl;
                    else
                        cout << "ALUout		" << (tmp->store_reg).store_value << endl;
                }
                cout << "WriteData	" << cur_register[tmp->rt] << endl;
                if(tmp -> op == R)
                    cout << "Rt/Rd		" << tmp -> rd<< endl;
                else
                    cout << "Rt/Rd		" << tmp -> rt<< endl;

                cout << "Control signals ";
                if(tmp -> type == R)
                    cout << "00010" << endl;
                else if(tmp -> type == I)
                {
                    if(tmp->op == LW)
                        cout << "01011" <<endl;
                    else if(tmp->op == SW)
                        cout << "00100" << endl;
                    else if(tmp->op == BEQ)
                        cout << "10000" << endl;
                    else if(tmp->op == ADDI)
                        cout << "00010" << endl;
                    else if(tmp->op == ANDI)
                        cout << "00010" << endl;
                }
            }
            else
            {
                cout << "EX/MEM :" << endl;
                cout << "ALUout		0" << endl;
                cout << "WriteData	0" << endl;
                cout << "Rt/Rd		0" << endl;
                cout << "Control signals 00000" << endl;
            }

            cout << endl;

            /*lw hazard*/
            if(ex_mem.size() != 0 && id_ex.size() != 0)
            {
                if(lw_hazard == true)
                    lw_hazard = false;
                else if(ex_mem[0].op == LW)
                {
                    if((ex_mem[0].rt == id_ex[0].rt)|| (ex_mem[0].rt == id_ex[0].rs))
                    {
                        lw_hazard = true;
                    }
                }
            }

            /*mem*/
            if(cycle > 3 && ex_mem.size() != 0 && control[3] == 1)
            {
                class instruction_def *tmp = new class instruction_def();
                *tmp = ex_mem[0];
                if(tmp->type == I)
                {
                    if(tmp->op == SW)
                    {
                        me[(tmp->store_mem).memory_location] = cur_register[(tmp->store_mem).store_value/4];
                    }
                    else if(tmp->op == LW)
                    {
                        (tmp->store_reg).lw_memory = me[(tmp->store_reg).store_value/4];
                    }
                }

                if(tmp->type == R)
                {
                    cur_register[(tmp->store_reg).reg_location] = (tmp->store_reg).store_value;
                }
                else if(tmp->type == I)
                {
                    if(tmp->op == LW)
                    {
                        cur_register[(tmp->store_reg).reg_location] = (tmp->store_reg).lw_memory;
                    }
                    else if(tmp->op == ANDI || tmp->op == ADDI)
                    {
                        cur_register[(tmp->store_reg).reg_location] = (tmp->store_reg).store_value;
                    }
                }
                mem_wb.push_back(*tmp);
                ex_mem.erase(ex_mem.begin());



                /*print*/
                cout << "MEM/WB :" << endl;
                if(tmp->type == I && tmp->op == LW)
                    cout << "ReadData	" << me[(tmp->immediate + tmp->rs_value)/4]<< endl;
                else
                    cout << "ReadData	" << "0"<< endl;

                if(tmp->type == R)
                {
                    cout << "ALUout		" << (tmp->store_reg).store_value << endl;
                }
                else if(tmp->type == I)
                {
                    if(tmp->op == SW)
                        cout << "ALUout		" << (tmp->store_mem).memory_location << endl;
                    else if(tmp->op == BEQ)
                        cout << "ALUout		" << tmp->beq_sol << endl;
                    else
                        cout << "ALUout		" << (tmp->store_reg).store_value << endl;
                }

                if(tmp -> op == R)
                    cout << "Rt/Rd		" << tmp -> rd<< endl;
                else
                    cout << "Rt/Rd		" << tmp -> rt<< endl;

                cout << "Control signals ";
                if(tmp -> type == R)
                    cout << "10" << endl;
                else if(tmp -> type == I)
                {
                    if(tmp->op == LW)
                        cout << "11" <<endl;
                    else if(tmp->op == SW)
                        cout << "00" << endl;
                    else if(tmp->op == BEQ)
                        cout << "00" << endl;
                    else if(tmp->op == ADDI)
                        cout << "10" << endl;
                    else if(tmp->op == ANDI)
                        cout << "10" << endl;
                }
            }
            else
            {
                cout << "MEM/WB :" << endl;
                cout << "ReadData	0" << endl;
                cout << "ALUout		0" << endl;
                cout << "Rt/Rd		0" << endl;
                cout << "Control signals 00" << endl;
            }


            cout << endl;

            /*data hazard check*/
            if(mem_wb.size() != 0 && id_ex.size() != 0 )
            {

                if((mem_wb[0].type == R && id_ex[0].type == R))
                {
                    if((mem_wb[0].rd == id_ex[0].rs))
                    {
                        id_ex[0].rs_value = mem_wb[0].store_reg.store_value;
                    }
                    else if((mem_wb[0].rd == id_ex[0].rt))
                    {
                        id_ex[0].rt_value = mem_wb[0].store_reg.store_value;
                    }

                }
                /*i-type*/
                else if(id_ex[0].type == I && mem_wb[0].type == I)
                {
                    if(mem_wb[0].op == LW || mem_wb[0].op == ADDI || mem_wb[0].op == ANDI)
                    {
                        if(id_ex[0].op == LW || id_ex[0].op == ADDI || id_ex[0].op == ANDI)
                        {
                            if(mem_wb[0].rt == id_ex[0].rs)
                                mem_wb[0].rs_value = mem_wb[0].store_reg.store_value;
                        }
                        else
                        {
                            if(mem_wb[0].rt == id_ex[0].rs)
                                id_ex[0].rs_value = mem_wb[0].store_reg.store_value;
                            if(mem_wb[0].rt == id_ex[0].rt)
                                id_ex[0].rt_value = mem_wb[0].store_reg.store_value;
                        }
                    }
                }
                else if(id_ex[0].type == R && mem_wb[0].type == I)
                {
                    if(mem_wb[0].op == LW || mem_wb[0].op == ADDI || mem_wb[0].op == ANDI)
                    {
                        if(mem_wb[0].rt == id_ex[0].rs)
                            id_ex[0].rs_value = mem_wb[0].store_reg.store_value;
                        if(mem_wb[0].rt == id_ex[0].rt)
                            id_ex[0].rt_value = mem_wb[0].store_reg.store_value;
                    }
                }
                else if(id_ex[0].type == I && mem_wb[0].type == R)
                {
                    if(id_ex[0].op == LW || id_ex[0].op == ADDI || id_ex[0].op == ANDI)
                    {
                        if(mem_wb[0].rd == id_ex[0].rs)
                            id_ex[0].rs_value = mem_wb[0].store_reg.store_value;
                    }
                    else
                    {
                        if(mem_wb[0].rd == id_ex[0].rs)
                            id_ex[0].rs_value = mem_wb[0].store_reg.store_value;
                        if(mem_wb[0].rd == id_ex[0].rt)
                            id_ex[0].rt_value = mem_wb[0].store_reg.store_value;
                    }
                }
            }

            /*wb*/
            if(cycle > 4 && control[4] == 1 && mem_wb.size()>0)
            {
                class instruction_def *tmp = new class instruction_def();
                *tmp = mem_wb[0];
                mem_wb.erase(mem_wb.begin());
            }
            cycle ++;
            control.pop_back();

            /*data hazard*/
            if(id_ex.size() != 0 && ex_mem.size() != 0)
            {
                if((id_ex[0].type == R && ex_mem[0].type == R) && ex_mem[0].rd != 0)
                {

                    if((id_ex[0].rs == ex_mem[0].rd))
                    {
                        id_ex[0].rs_value = ex_mem[0].store_reg.store_value;
                    }
                    else if((id_ex[0].rt == ex_mem[0].rd))
                    {
                        id_ex[0].rt_value = ex_mem[0].store_reg.store_value;
                    }
                }
                /*i-type*/
                else if(id_ex[0].type == I && ex_mem[0].type == I)
                {
                    if(ex_mem[0].op == LW || ex_mem[0].op == ADDI || ex_mem[0].op == ANDI)
                    {
                        if(id_ex[0].op == LW || id_ex[0].op == ADDI || id_ex[0].op == ANDI)
                        {
                            if(ex_mem[0].rt == id_ex[0].rs)
                                id_ex[0].rs_value = ex_mem[0].store_reg.store_value;
                        }
                        else
                        {
                            if(ex_mem[0].rt == id_ex[0].rs)
                                id_ex[0].rs_value = ex_mem[0].store_reg.store_value;
                            if(ex_mem[0].rt == id_ex[0].rt)
                                id_ex[0].rt_value = ex_mem[0].store_reg.store_value;
                        }
                    }
                }
                else if(id_ex[0].type == R && ex_mem[0].type == I)
                {
                    if(ex_mem[0].op == LW || ex_mem[0].op == ADDI || ex_mem[0].op == ANDI)
                    {
                        if(ex_mem[0].rt == id_ex[0].rs)
                            id_ex[0].rs_value = ex_mem[0].store_reg.store_value;
                        if(ex_mem[0].rt == id_ex[0].rt)
                            id_ex[0].rt_value = ex_mem[0].store_reg.store_value;
                    }
                }
                else if(id_ex[0].type == I && ex_mem[0].type == R)
                {
                    if(id_ex[0].op == LW || id_ex[0].op == ADDI || id_ex[0].op == ANDI)
                    {
                        if(ex_mem[0].rd == id_ex[0].rs)
                            id_ex[0].rs_value = ex_mem[0].store_reg.store_value;
                    }
                    else
                    {
                        if(ex_mem[0].rd == id_ex[0].rs)
                            id_ex[0].rs_value = ex_mem[0].store_reg.store_value;
                        if(ex_mem[0].rd == id_ex[0].rt)
                            id_ex[0].rt_value = ex_mem[0].store_reg.store_value;
                    }
                }
            }
            cout << "=================================================================" << endl;
        }
        cout.close();
        printf("%s is finished\n", input_file_name[i].c_str());
    }
}
