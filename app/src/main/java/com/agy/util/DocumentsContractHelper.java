package com.agy.util;

import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.util.Log;

import java.io.File;

/**
 * Resolves Storage Access Framework (SAF) tree URIs to real filesystem paths.
 */
public final class DocumentsContractHelper {

    private static final String TAG = "DocumentsContractHelper";

    private DocumentsContractHelper() {
    }

    public static String resolveTreeUriToPath(Context context, Uri treeUri) {
        if (treeUri == null) {
            return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath();
        }
        try {
            String documentId = DocumentsContract.getTreeDocumentId(treeUri);
            if (documentId == null) {
                return Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS).getAbsolutePath();
            }
            String baseStorage = Environment.getExternalStorageDirectory().getAbsolutePath();
            File downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);

            // Downloads provider raw path format (e.g., "raw:/storage/emulated/0/Download/...")
            if (documentId.startsWith("raw:")) {
                return documentId.substring("raw:".length());
            }

            // Downloads provider root or subpath format
            if ("downloads".equalsIgnoreCase(documentId)) {
                return downloadsDir != null ? downloadsDir.getAbsolutePath() : new File(baseStorage, "Download").getAbsolutePath();
            }
            if (documentId.toLowerCase().startsWith("downloads:")) {
                String subPath = documentId.substring("downloads:".length());
                if (subPath.startsWith("/")) {
                    subPath = subPath.substring(1);
                }
                return new File(downloadsDir, subPath).getAbsolutePath();
            }

            // Format is often "primary:Download/my-project" or "<volume-uuid>:folder"
            if (documentId.startsWith("primary:")) {
                String subPath = documentId.substring("primary:".length());
                if (subPath.startsWith("/")) {
                    subPath = subPath.substring(1);
                }
                return subPath.isEmpty() ? baseStorage : new File(baseStorage, subPath).getAbsolutePath();
            }

            String[] parts = documentId.split(":");
            if (parts.length > 1) {
                String volume = parts[0];
                String subPath = parts[1];
                if (subPath.startsWith("/")) {
                    subPath = subPath.substring(1);
                }
                if ("primary".equalsIgnoreCase(volume)) {
                    return subPath.isEmpty() ? baseStorage : new File(baseStorage, subPath).getAbsolutePath();
                }
                File external = new File("/storage/" + volume + "/" + subPath);
                if (external.exists()) {
                    return external.getAbsolutePath();
                }
                return subPath.isEmpty() ? baseStorage : new File(baseStorage, subPath).getAbsolutePath();
            }

            String cleanDocId = documentId.startsWith("/") ? documentId.substring(1) : documentId;
            return cleanDocId.isEmpty() ? baseStorage : new File(baseStorage, cleanDocId).getAbsolutePath();
        } catch (Throwable t) {
            Log.w(TAG, "Error resolving tree uri to path: " + treeUri, t);
            File downloadsDir = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOWNLOADS);
            return downloadsDir != null ? downloadsDir.getAbsolutePath() : "/storage/emulated/0/Download";
        }
    }
}
