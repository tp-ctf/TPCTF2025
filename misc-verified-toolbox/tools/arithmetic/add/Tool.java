public class Tool {
    public static String run(String input) {
        try {
            String[] numbers = input.split(" ");
            int a = Integer.parseInt(numbers[0]);
            int b = Integer.parseInt(numbers[1]);
            return String.valueOf(a + b);
        } catch (Exception e) {
            return "Invalid input";
        }
    }
}
