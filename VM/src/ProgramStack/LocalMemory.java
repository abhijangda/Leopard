package ProgramStack;
import java.util.*;

//Little Endian, least significant byte in the smallest address.

public class LocalMemory 
{
	Stack<Object> memory;
	
	public LocalMemory ()
	{
		memory = new Stack<Object> ();
	}
	
	public void push (Object value)
	{
		memory.push(value);
	}
}
