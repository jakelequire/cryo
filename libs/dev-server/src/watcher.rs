// src/watcher.rs
use std::{
    fs,
    path::{Path, PathBuf},
    process::Command,
    thread,
    time::Duration,
    collections::HashMap,
    sync::atomic::{AtomicBool, Ordering},
    sync::Arc,
};

pub struct FileWatcher {
    files: HashMap<PathBuf, std::time::SystemTime>,
    ignore_dirs: Vec<&'static str>,
    project_path: PathBuf,
    running: Arc<AtomicBool>,
}

impl FileWatcher {
    pub fn new(project_path: &Path, ignore_dirs: Vec<&'static str>) -> std::io::Result<Self> {
        let mut watcher = FileWatcher {
            files: HashMap::new(),
            ignore_dirs,
            project_path: project_path.to_path_buf(),
            running: Arc::new(AtomicBool::new(true)),
        };
        watcher.update_files(project_path)?;
        Ok(watcher)
    }

    pub fn stop(&self) {
        self.running.store(false, Ordering::SeqCst);
    }

    fn should_ignore(&self, path: &Path) -> bool {
        for ignore_dir in &self.ignore_dirs {
            if path.to_string_lossy().contains(ignore_dir) {
                return true;
            }
        }
        false
    }

    fn update_files(&mut self, dir: &Path) -> std::io::Result<()> {
        if self.should_ignore(dir) {
            return Ok(());
        }

        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();

            if path.is_dir() {
                self.update_files(&path)?;
            } else {
                let metadata = fs::metadata(&path)?;
                self.files.insert(path, metadata.modified()?);
            }
        }
        Ok(())
    }

    fn check_for_changes(&mut self, dir: &Path) -> std::io::Result<bool> {
        let mut new_files = HashMap::new();
    
        // Directly return the result of check_dir
        let changed = Self::check_dir(dir, &mut new_files, &self.files, &self.ignore_dirs)?;
    
        // Check for deleted files
        let deleted_files_exist = self.files.keys().any(|old_path| !new_files.contains_key(old_path));
    
        self.files = new_files;
        Ok(changed || deleted_files_exist)
    }
    
    pub fn watch_directory(&mut self) -> std::io::Result<()> {
        // Initial build
        self.rebuild_project()?;
    
        let project_path = self.project_path.clone();
        while self.running.load(Ordering::SeqCst) {
            if self.check_for_changes(&project_path)? {
                self.rebuild_project()?;
            }
            thread::sleep(Duration::from_secs(1));
        }
        
        Ok(())
    }

    fn rebuild_project(&self) -> std::io::Result<()> {
        println!("Changes detected, rebuilding project...");
        
        let output = Command::new("make")
            .current_dir(&self.project_path)
            .output()?;
        
        if output.status.success() {
            println!("Build successful!");
            println!("{}", String::from_utf8_lossy(&output.stdout));
        } else {
            eprintln!("Build failed!");
            eprintln!("{}", String::from_utf8_lossy(&output.stderr));
        }
        
        Ok(())
    }

    fn check_dir(
        dir: &Path,
        new_files: &mut HashMap<PathBuf, std::time::SystemTime>,
        old_files: &HashMap<PathBuf, std::time::SystemTime>,
        ignore_dirs: &[&str],
    ) -> std::io::Result<bool> {
        if ignore_dirs.iter().any(|ignore_dir| dir.ends_with(ignore_dir)) {
            return Ok(false);
        }
    
        let mut changed = false;
    
        for entry in fs::read_dir(dir)? {
            let entry = entry?;
            let path = entry.path();
    
            if path.is_dir() {
                if Self::check_dir(&path, new_files, old_files, ignore_dirs)? {
                    changed = true;
                }
            } else {
                let metadata = fs::metadata(&path)?;
                let modified = metadata.modified()?;
    
                if let Some(&old_modified) = old_files.get(&path) {
                    if modified != old_modified {
                        changed = true;
                    }
                } else {
                    changed = true;
                }
    
                new_files.insert(path, modified);
            }
        }
    
        Ok(changed)
    }
}