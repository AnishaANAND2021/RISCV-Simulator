from tkinter import *
from tkinter import ttk
import subprocess

root = Tk()
root.title("CS204 PROJECT")

# Class to take the input instructions and save them in the input.text file
class Application(Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_widgets()

    def create_widgets(self):
        self.instr = Label(self, text="Enter the instructions")
        self.instr.pack()

        self.instructions = Text(self, height=7, width=100, bg = "light cyan")
        self.instructions.pack()

        self.file_button = Button(self, text="Save to Input File", command=self.save_to_file)
        self.file_button.pack()

    def save_to_file(self):
        inst = self.instructions.get('1.0', 'end')
        with open("input.txt", 'w') as f:
            f.write(inst)

# Class to compile the c++ program and provide the output
class CppRunnerGUI(Frame):
    def __init__(self, master=None):
        super().__init__(master)
        self.master = master
        self.pack()
        self.create_widgets()

    def create_widgets(self):
        # Create a button to compile and run the C++ program
        self.run_button = Button(self)
        self.run_button["text"] = "Compile and Run C++ Program"
        self.run_button["command"] = self.compile_and_run_cpp_program
        self.run_button.pack(side="top")

        # Create a text box to display the output from the C++ program
        self.output_text = Text(self, height = 7, width = 100, bg = "light yellow")
        self.output_text.pack()

    def compile_and_run_cpp_program(self):
<<<<<<< HEAD
        compile_process = subprocess.Popen(["g++", "a.cpp", "-o", "a.exe"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
=======
        compile_process = subprocess.Popen(["g++", "phase2.cpp", "-o", "a.exe"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
>>>>>>> 0e0d3de0f74db8e6666c43364640bda7360138c3

        # Wait for the compilation process to finish and capture the output and error messages (if any)
        compile_output, compile_error = compile_process.communicate()

        # Print the output and error messages (if any)
        self.output_text.insert(END, compile_output.decode())
        self.output_text.insert(END, compile_error.decode())

        # If the compilation was successful, execute the compiled program
        if compile_process.returncode == 0:
            run_process = subprocess.Popen(["a.exe"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            # Wait for the execution process to finish and capture the output and error messages (if any)
            run_output, run_error = run_process.communicate()

            # Print the output and error messages (if any)
            self.output_text.insert(END, run_output.decode())
            self.output_text.insert(END, run_error.decode())

app = Application(master=root)
app = CppRunnerGUI(master=root)


output = Tk()
output.title("Output files")
output.geometry("600x800")

main_frame = Frame(output)
main_frame.pack(fill=BOTH, expand=1)

my_canvas = Canvas(main_frame)
my_canvas.pack(side=LEFT, fill=BOTH, expand=1)

my_scrollbar = ttk.Scrollbar(main_frame, orient = VERTICAL, command= my_canvas.yview)
my_scrollbar.pack(side=RIGHT, fill=Y)

my_canvas.configure(yscrollcommand = my_scrollbar.set)
my_canvas.bind('<Configure>', lambda e: my_canvas.configure(scrollregion = my_canvas.bbox("all")))

second_frame = Frame(my_canvas)

my_canvas.create_window((0,0), window = second_frame, anchor = "nw")

# function to open memory file
def open_memory_file():
    with open("Memory_file.txt", 'r') as f:
        contents1 = f.read()
        text1.delete('1.0', END)
        text1.insert(END, contents1)

open_button1 = Button(second_frame, text="Open Memory File", command=open_memory_file)
open_button1.pack()
text1 = Text(second_frame, height =20, width=60, bg = "light yellow" )
text1.pack()

# function to open register file with decimal values
def open_register_file_dec():
    with open("register_file_dec.txt", 'r') as f:
        contents2 = f.read()
        text2.delete('1.0', END)
        text2.insert(END, contents2)

open_button2 = Button(second_frame, text="Open Register File (dec)", command=open_register_file_dec)
open_button2.pack()
text2 = Text(second_frame, height =20, width=60, bg = "light green")
text2.pack()

# function to open register file with hexadecimal values
def open_register_file_hex():
    with open("register_file_hex.txt", 'r') as f:
        contents3 = f.read()
        text3.delete('1.0', END)
        text3.insert(END, contents3)

open_button3 = Button(second_frame, text="Open Register File (hex)", command=open_register_file_hex)
open_button3.pack()
text3 = Text(second_frame, height =20, width=60, bg = "light yellow")
text3.pack()

# function to open terminal file
def open_terminal_file():
    with open("terminal.txt", 'r') as f:
        contents4 = f.read()
        text4.delete('1.0', END)
        text4.insert(END, contents4)

open_button4 = Button(second_frame, text="Open Terminal File", command=open_terminal_file)
open_button4.pack()
text4 = Text(second_frame, height =20, width=80, bg = "light green")
text4.pack()

# Adding the block diagram of the five pipeline stages
photo1 = PhotoImage(file = "pipeline.png")
pipelining_label = Label(image = photo1)
pipelining_label.pack()


output.mainloop()
app.mainloop()
root.mainloop()

