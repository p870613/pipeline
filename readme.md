實作指令: lw, sw, add, addi, sub, and, andi, or, slt, beq 
偵測與處理: data hazard, hazard with load, branch hazard

- input:
	MIPS 指令轉換後的 32-bit 機器碼指令為程式的輸入
- output:
	根據連續輸入的 32-bit 機器碼指令，模擬其在每個 clock cycle 時，各個 pipeline registers
所儲存的值，並輸出寫入至對應文件中。

- 程式會自行讀檔，讀檔檔名必須是General、 Datahazard、lwhazard、Branchhazard

- 程式會輸出檔案，檔案分別為genResult.txt、dataResult.txt、loadResult.txt、branchResult.txt

- 如需改檔名，請改程式58、59行，
   58行為input_file的檔名
   59行為output_file的檔名
