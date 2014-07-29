package ProgramStack;
import java.util.*;

public class ProgramStack extends Stack<ActivationRecord> 
{
	LocalMemory globalMemory;
	ActivationRecord currentRecord;

	public ProgramStack ()
	{
		super ();
		globalMemory = new LocalMemory ();
	}
	
	public void callFunction ()
	{
		currentRecord = new ActivationRecord (currentRecord.pc, globalMemory, 
											  currentRecord);
		push (currentRecord);
	}
	
	public ActivationRecord getCurrentRecord ()
	{
		return currentRecord;
	}
	
	public void retFunction ()
	{
		pop ();
		currentRecord = peek ();
	}
}
