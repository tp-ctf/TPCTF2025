https://ouuan.moe/post/2025/03/tpctf-2025

## verified toolbox (1 solve)

It uses Spring Boot 3.3.2, so it’s [CVE-2024-38807: Signature Forgery Vulnerability in Spring Boot’s Loader](https://spring.io/security/cve-2024-38807)[^finder].

[^finder]: This CVE is found by me.

The vulnerability is that spring-boot-loader uses `JarInputStream` to verify the signatures but uses a custom `ZipContent` class to load the contents. They parse a ZIP file differently and may read different contents from a specially crafted JAR file. `JarInputStream` reads a JAR file from start to end, while `ZipContent` read the end of central directory record at the end first. We can construct a malicious JAR file by concatenating the bytes of two JAR files, and then adjust the offset fields in the central directory headers and the end of central directory record of the second JAR file. The signature verifier will read the first JAR file while the content loader will read the second.

You can also find [the commit that fixes this vulnerability](https://github.com/spring-projects/spring-boot/commit/0b24ee857189e139f48826bf2aef10ae8680c11b) along with the `mismatched.jar` test case, and then create the malicious JAR file based on `mismatched.jar`.
