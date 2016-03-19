package me.haved.daf;

import java.io.File;
import java.util.ArrayList;

import static me.haved.daf.LogHelper.*;

public class RegisteredFile {
	
	private static ArrayList<RegisteredFile> files = new ArrayList<RegisteredFile>();
	
	public File fileObject;
	public String fileName;
	public String canonicalPath;
	
	private int id;
	
	private RegisteredFile(File file, String fileName, String canonicalPath) {
		this.fileObject = file;
		this.fileName = fileName;
		this.canonicalPath = canonicalPath;
		files.add(this);
		id = files.indexOf(this); //There should(must) be a prettier way
	}
	
	public int getId() {
		return id;
	}
	
	public String getErrorString() {
		return fileName;
	}
	
	public String toString() {
		return  String.format("RegisteredFile(fileObject:\"%s\", fileName:\"%s\", id:%d)", fileObject.toString(), fileName, id);
	}
	
	
	/** Get a RegisteredFile instance created for this file, or an old one if there already is one with the same canonical path
	 * 
	 * @param file A file that must exist
	 * @param fileName The error name
	 * @return The new or old instance
	 */
	public static RegisteredFile registerNewFile(File file, String fileName) {
		String canonicalPath = getCanonicalPath(file);
		
		RegisteredFile alreadyReg = getAlreadyRegisteredFile(canonicalPath);
		if(alreadyReg!=null) {
			log(MESSAGE, "The file '%s' was already registered! Returning that one!", file.toString());
			if(!alreadyReg.fileName.equals(fileName))
				log(WARNING, "The already registered file has the fileName '%s', and not the new '%s'!", alreadyReg.fileName, fileName);
			return alreadyReg;
		}
		return new RegisteredFile(file, fileName, canonicalPath);
	}
	
	public static RegisteredFile getAlreadyRegisteredFile(File file) {
		return getAlreadyRegisteredFile(getCanonicalPath(file));
	}
	
	public static RegisteredFile getAlreadyRegisteredFile(String canonicalPath) {
		for(int i = 0; i < files.size(); i++) {
			if(files.get(i).canonicalPath.equals(canonicalPath))
				return files.get(i); //A well
		}
		return null;
	}
	
	public static RegisteredFile getRegisteredFile(int id) {
		if(id >= files.size())
			return null;
		return files.get(id);
	}
	
	public static String getCanonicalPath(File file) { //The file must exist at this point!
		if(file == null)
			log(ASSERTION_FAILED, "Tried to get canonical path of null file!");
		if(!file.isFile())
			log(ASSERTION_FAILED, "File passed to getCanonicalPath doesn't exist!");
		try {
			return file.getCanonicalPath();
		} catch(Exception e) {
			log(ASSERTION_FAILED, "Failed to get canonical path of existing file '%s'", file.toString());
			return null;
		}
	}
}
