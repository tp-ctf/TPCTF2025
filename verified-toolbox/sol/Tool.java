import java.io.ByteArrayOutputStream;

public class Tool {
    public static String run(String cmd) {
        try {
            ProcessBuilder processBuilder = new ProcessBuilder("sh", "-c", cmd);
            Process process = processBuilder.start();

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            bos.write(String.format("\n$ %s\n", cmd).getBytes());

            process.getInputStream().transferTo(bos);
            process.getErrorStream().transferTo(bos);

            return bos.toString();
        } catch (Exception e) {
            return e.getMessage();
        }
    }
}
