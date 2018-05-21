public class ReturnValue {
	public static void printInt(int value) {
		System.out.println(value);
	}

	public static int getInt()
	{
		return 42;
	}

	public static void main(String[] args) {
		int x = getInt();
		printInt(x);
	}

}
