/*
	This is just a skeleton. It DOES NOT implement all the requirements.
	It only recognizes the "ADD", "SUB" and "ADDI"instructions and prints
	"Unkown Instruction" for all other instructions!
 
	References:
	(1) The risc-v ISA Manual ver. 2.1 @ https://riscv.org/specifications/
	(2) https://github.com/michaeljclark/riscv-meta/blob/master/meta/opcodes
 */

#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>

using namespace std;

int regs[32]={0};
unsigned int pc = 0x0;

char memory[8*1024];	// only 8KB of memory located at address 0

void emitError(char *s)
{
    cout << s;
    exit(0);
}

void printPrefix(unsigned int instA, unsigned int instW){
    cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}
void instDecExec(unsigned int instWord)
{
    unsigned int rd, rs1, rs2, funct3, funct7, opcode;
    unsigned int I_imm, S_imm, B_imm, U_imm, J_imm;
    unsigned int address;
    regs[0] = 0 ;
    unsigned int instPC = pc - 4;
    
    opcode = instWord & 0x0000007F;
    rd = (instWord >> 7) & 0x0000001F;
    funct3 = (instWord >> 12) & 0x00000007;
    rs1 = (instWord >> 15) & 0x0000001F;
    rs2 = (instWord >> 20) & 0x0000001F;
    funct7 =(instWord >> 25) & 0x00001F;
    
    // — inst[31] — inst[30:25] inst[24:21] inst[20]
    I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
    S_imm = ((funct7<<7) | rd)|(((instWord >> 31) ? 0xFFFFF800 : 0x0));
    U_imm = (instWord & 0xFFFFF000);
    B_imm = ((rd & 0x1E)<<1) | ((funct7 & 0x3F) << 5) | ((rd& 0x1) << 11) | (instWord & 0x80000000) << 31;
    J_imm = ((instWord& 0x7FE00000) >> 20 ) | ((instWord >> 20 & 0x1) << 11) | ((instWord >> 12 & 0x7F) << 12)
    | (((instWord >> 31) ? 0xFFF00000 : 0x0));
    

    
    printPrefix(instPC, instWord);
    
    if(opcode == 0x33){		// R Instructions
        switch(funct3){
            case 0: if(funct7 == 32) {
                cout << "\tSUB\tx" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
                regs[rd] = regs[rs1] - regs[rs2];
            }
            else {
                cout << "\tADD\tx" << rd << ", x" << rs1 << ", x" << rs2 << "\n";
                regs[rd] = regs[rs1] + regs[rs2];
            }
                break;
            default:
                cout << "\tUnkown R Instruction \n";
        }
    } else if(opcode == 0x13){	// I instructions
        switch(funct3){
            case 0:	cout << "\tADDI\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
                regs[rd] = regs[rs1] + (int)I_imm;
                break;
            default:
                cout << "\tUnkown I Instruction \n";
        }
    } else if(opcode == 0x37) //LUI
    {
        cout << "\tLUI\tx" << rd << ", " << hex << "0x" << (int)U_imm << "\n";
        regs[rd] = U_imm;
    }
    else if(opcode == 0x17) // AUIPC
    {
        cout << "\tAUIPC\tx" << rd << ", " << hex << "0x" << (int)U_imm << "\n";
        regs[rd] = U_imm + instPC;
    }
    
    else if(opcode == 0x6F) // JAL
    {
        cout << "\tJAL\tx" << rd << ", " << hex << "0x" << (int)J_imm << "\n";
        regs[rd] = pc;
        pc += (int)J_imm;
    }
    
    else if(opcode == 0x67) // JALR
    {
        cout << "\tJALR\tx" << rd << ", x" << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
        regs[rd] = pc;
        pc = (I_imm + rs1)&0xFFFFFFFE;
    }

    else if(opcode == 0x63) // Branch Instructions
    {
        
        switch(funct3)
        {
                
            case 0:
                cout << "\tBEQ\tx" << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n";
                if(rs1==rs2)
                    pc+= (int)B_imm;
                break;
                
            case 1:
                cout << "\tBNE\tx" << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n";
                if(rs1!=rs2)
                    pc+= (int)B_imm;
                break;
                
                
            case 4:
                cout << "\tBLT\tx" << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n";
                if((int)rs1<(int)rs2)
                    pc+= (int)B_imm;
                break;
                
                
            case 5:
                cout << "\tBGE\tx" << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n";
                if((int)rs1>=(int)rs2)
                    pc+= (int)B_imm;
                break;
                
                
            case 6:
                cout << "\tBLTU\tx" << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n";
                if(rs1<rs2)
                    pc+= (int)B_imm;
                break;
                
                
            case 7:
                cout << "\tBGEU\tx" << rs1 << ", x" << rs2 << ", " << hex << "0x" << (int)B_imm << "\n";
                if(rs1>=rs2)
                    pc+= (int)B_imm;
                break;
                
                
                default:
                cout << "UNKOWN SB INSTRUCTION " << endl;
        }
    
    }

    
    else if(opcode == 0x3) //LOADING INSTRUCTIONS
    {
        switch(funct3)
        {
                
            case 0:
                cout << "\tLB\tx" << rd << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                regs[rd] = 0;
                regs[rd] = (unsigned char)memory[rs1 + (int)I_imm]  | ((( (unsigned char) memory[rs1 + (int)I_imm] >> 7) ? 0xFFFFFF00 : 0x0));
                
                break;
                
                
            case 1:
                
                cout << "\tLH\tx" << rd << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                regs[rd] = 0;
                regs[rd] = ((unsigned char) memory[rs1 + (int)I_imm]) | ((unsigned char) memory[rs1 + (int)I_imm+1] << 8)
                | (((((unsigned char) memory[rs1 + (int)I_imm +1]) >> 15) ? 0xFFFF0000 : 0x0));
                

                break;
                
                
            case 2:
                cout << "\tLW\tx" << rd << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                regs[rd] = ((unsigned char) memory[rs1 + (int)I_imm]) | (((unsigned char) memory[rs1 + (int)I_imm+1]) << 8)
                | (((unsigned char) memory[rs1 + (int)I_imm+2]) << 16)
                | (((unsigned char) memory[rs1 + (int)I_imm+3]) << 24) ;
                break;
                
                
            case 4:
                cout << "\tLBU\tx" << rd << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                regs[rd] = 0;
                regs[rd] = (unsigned char)memory[rs1 + (int)I_imm];
                break;
                
                
            case 5:
                cout << "\tLHU\tx" << rd << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                regs[rd] = 0;
                regs[rd] = ((unsigned char) memory[rs1 + (int)I_imm]) | ((unsigned char) memory[rs1 + (int)I_imm+1] << 8);
                break;
                
                
                default:
                cout<<"Uknown Load Instruction" <<endl;
                
                
                
                
                
        }
     
        
        
    }
    
    else if(opcode == 0x23) // STORE INSTRUCTIONS
    {
        switch(funct3)
        {
            case 0:
                cout << "\tSB\tx" << rs2 << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                memory[rs1 + S_imm] = regs[rs2];
                break;
                
            case 1:
                cout << "\tSH\tx" << rs2 << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                memory[rs1 + S_imm] = regs[rs2];
                memory[rs1 + S_imm+1] = regs[rs2]>>8;

                break;
            
            case 2:
                
                cout << "\tSW\tx" << rs2 << ", " << hex << "0x" << (int)I_imm  << "(x" << rs1 <<")" << "\n";
                memory[rs1 + S_imm] = regs[rs2];
                memory[rs1 + S_imm+1] = regs[rs2]>>8;
                memory[rs1 + S_imm+2] = regs[rs2]>>16;
                memory[rs1 + S_imm+3] = regs[rs2]>>24;
                break;
                
                
                default:
                cout<<"UKNOWN STORE INSTRUCTION" << endl ;
                
                
        }
        
    }
    
    
    else {
        cout << "\tUnkown Instruction \n";
    }
    
}

int main(int argc, char *argv[]){
    
    unsigned int instWord=0;
    ifstream inFile;
    ofstream outFile;
    string x = "/Users/ahmadrefaat/Documents/Assembly/Assembly Project/RISCV/RISCV/div.bin";
    //if(argc<1) emitError("use: rv32i_sim <machine_code_file_name>\n");
    
    //inFile.open(argv[1], ios::in | ios::binary | ios::ate);
    inFile.open(x.c_str(), ios::in | ios::binary | ios::ate);
    
    if(inFile.is_open())
    {
        int fsize = inFile.tellg();
        
        inFile.seekg (0, inFile.beg);
        if(!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");
        
        while(true){
            instWord = 	(unsigned char)memory[pc] |
            (((unsigned char)memory[pc+1])<<8) |
            (((unsigned char)memory[pc+2])<<16) |
            (((unsigned char)memory[pc+3])<<24);
            pc += 4;
            // remove the following line once you have a complete simulator
            if(pc==32) break;			// stop when PC reached address 32
            instDecExec(instWord);
        }
        
        // dump the registers
        for(int i=0;i<32;i++)
            cout << "x" << dec << i << ": \t"<< "0x" << hex << std::setfill('0') << std::setw(8) << regs[i] << "\n";
        
    } else emitError("Cannot access input file\n");
}
