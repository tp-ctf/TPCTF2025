package org.tpctf.verified_toolbox;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.regex.Pattern;
import org.springframework.boot.loader.jar.NestedJarFile;
import org.springframework.core.io.ClassPathResource;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

@RestController
public class ToolboxRunner {
    private static final int MAX_FILE_SIZE = 100 * 1024;
    private static final int MAX_INPUT_LENGTH = 100;
    private static final Pattern JAR_PATH_PATTERN = Pattern.compile("^[a-zA-Z]+\\.jar$");
    private static final Pattern NO_SIGNATURE_PATTERN = Pattern.compile("^META-INF/([A-Z]+\\.(MF|SF|DSA))?$");
    private static final Certificate SIGNATURE_CERT;

    static {
        try (InputStream is = new ClassPathResource("signature.crt").getInputStream()) {
            CertificateFactory cf = CertificateFactory.getInstance("X.509");
            SIGNATURE_CERT = cf.generateCertificate(is);
        } catch (Exception e) {
            throw new RuntimeException("Failed to load signature certificate", e);
        }
    }

    @PostMapping("/run")
    public ResponseEntity<String> runToolbox(
        @RequestParam("file") MultipartFile file,
        @RequestParam("path") String path,
        @RequestParam("input") String input
    ) {
        if (file.getSize() > MAX_FILE_SIZE) {
            return ResponseEntity.badRequest().body("File too large (max 100KB)");
        }
        if (!JAR_PATH_PATTERN.matcher(path).matches()) {
            return ResponseEntity.badRequest().body(String.format("Inner JAR path must match /%s/", JAR_PATH_PATTERN.pattern()));
        }
        if (input.length() > MAX_INPUT_LENGTH) {
            return ResponseEntity.badRequest().body("Input too long (max 100 characters)");
        }

        File nestedJarFile = null;

        try {
            try {
                nestedJarFile = File.createTempFile("nested-jar", ".jar");
                file.transferTo(nestedJarFile);
            } catch (Exception e) {
                return ResponseEntity.internalServerError().body("Internal Error");
            }

            if (!validateSignature(nestedJarFile, path)) {
                return ResponseEntity.badRequest().body("Try harder!");
            }

            String nestedJarUrl = String.format("jar:nested:%s/!%s!/", nestedJarFile.getPath(), path);

            try (URLClassLoader classLoader = new URLClassLoader(new URL[] { new URL(nestedJarUrl) })) {
                Class<?> toolClass = classLoader.loadClass("Tool");
                Method run = toolClass.getMethod("run", String.class);
                String output = (String) run.invoke(null, input);
                return ResponseEntity.ok(output);
            }
        } catch (Exception e) {
            return ResponseEntity.badRequest().body("Try harder!!");
        } finally {
            if (nestedJarFile != null && nestedJarFile.exists()) {
                nestedJarFile.delete();
            }
        }
    }

    private static boolean validateSignature(File nestedJarFile, String innerJarPath) {
        try (NestedJarFile jar = new NestedJarFile(nestedJarFile, innerJarPath)) {
            Enumeration<JarEntry> entries = jar.entries();
            while (entries.hasMoreElements()) {
                JarEntry entry = entries.nextElement();
                if (NO_SIGNATURE_PATTERN.matcher(entry.getName()).matches()) {
                    continue;
                }
                try (InputStream is = jar.getInputStream(entry)) {
                    is.transferTo(OutputStream.nullOutputStream());
                }
                Certificate[] certs = entry.getCertificates();
                if (certs == null || certs.length != 1 || !certs[0].equals(SIGNATURE_CERT)) {
                    return false;
                }
            }
            return true;
        } catch (Exception e) {
            return false;
        }
    }
}
