public class Fib {
	public static void printInt(int value) {
		System.out.println(value);
	}

	public static int fibonacci(int n)
	{
		if(n < 2)
			return n;
		else 
			return fibonacci(n-1) + fibonacci(n-2);
			
	}

	public static void main(String[] args) {

		int x = fibonacci(12);
		printInt(x);
	}

}
