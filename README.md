# SIC - README
###### tags: `大二下` `系統程式`
There are two passes of my C program.

完整版詳見hackmd
[Hackmd_link](https://hackmd.io/knHmVIC6S_OyV-Rqa4x3Yw?both)

---
### Data
* Input data:
    * opcode.txt
        >It's is a txt of opcode
        >This is a part of **opcode.txt**
        >![](https://hackmd.io/_uploads/rkD676gvh.png)
        >.
        >.
        >.
    * sic.txt
        >This is the source input
        >This is a part of **sic.txt**
        >![](https://hackmd.io/_uploads/r144EalD3.png)
        >.
        >.
        >.
* Output file:
    * pass1_source_program.txt
        >This one is the file of source with addind the **location** column
    * symbol_table.txt
        >This is the **symbol table** generate as pass1 finish
    * pass2_source_program.txt
        >This is the file of source with addind the **location** and **object code** column
    * object_program.txt
        >This is the final program which can be run as a machine code,and also the **answer**

---
### Operation method
1.  Open the **SIC.c** in my file
2.  It will generate **four '.txt'**
3.  Check it and you will find they are correct!

---
### About the program logic

Use two struct to build **opcode** and **symbol table's** structs

* opcode
    * save opcode
        >easily to check the number of opcode
        >![](https://hackmd.io/_uploads/BJToD6gP3.png)
* pass1
    * Use ***data structure*** - stack
        >![](https://hackmd.io/_uploads/r1GNuTeDn.png)
        >
In pass1:
        ->first open file sic.txt
        ->use the **opcode table**
        ->while making a symbol table,it need to check the location
        ->and then,generate **pass1_source_program.txt**
        ->after,the symbol table will be built with their loction was known
* pass2
    * Use ***data structure*** - queue
        >![](https://hackmd.io/_uploads/Hkz5K6lPh.png)
        >
In pass2:
        ->first calculate the **object code** of each line
        ->and then,generate **pass2_source_program.txt**
        ->finally,**object_program** generated
        

