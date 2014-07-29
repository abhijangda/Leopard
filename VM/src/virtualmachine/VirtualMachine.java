package virtualmachine;
import ProgramStack.*;
import registers.*;
import java.util.*;

public class VirtualMachine 
{
	ProgramStack programStack;
	ProgramCounter programCounter;
	HashMap <Byte, Register> map;
	
	public VirtualMachine ()
	{
		programCounter = new ProgramCounter ();
		programStack = new ProgramStack ();
		map = new HashMap <Byte, Register> ();
		
	}
}
