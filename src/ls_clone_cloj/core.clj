(ns ls-clone-cloj.core
  (:require [clojure.tools.cli :refer [parse-opts]])
  (:require [clojure.java.io :as io])
  (:require [clojure.string :refer [join]])
  (:gen-class))

(def working-path-default ".")

(defn bail-out-with-usage []
  (binding [*out* *err*]
    (do
      (println (str "Usage: " (first *command-line-args*) " [-t / --tree] [dir]"))
      (System/exit 1)
    )
  )
)

(defn working-path-from-args 
  [args]
  (if (= (count args) 1)
    (if (or (= (first args) "-t") (= (first args) "--tree"))
      working-path-default
      (first args)
    )
    (let
      [last-entry (last args)]
      (if (or (= last-entry "-t") (= last-entry "--tree"))
        (bail-out-with-usage)
        last-entry
      )
    )
  )
)

(defn working-path [args]
  (if (> (count args) 0)
    (working-path-from-args args)
    working-path-default
  )
)

(defn configure 
  [args]
  (let 
    [spec [["-t" "--tree" "Display a formatted tree for a given directory" :default false]]
     { { is-tree-format :tree } :options } (parse-opts args spec)]
    { :is-tree-format is-tree-format :path (working-path args)} 
  )
)

(defn file-structure [java-file]
  { :name (.getName java-file) :is-directory (.isDirectory java-file) }
)

(defn formatted-file 
  [{ is-directory :is-directory name :name }]
  (str name (if is-directory "/" ""))
)

(defn shallow-directory-listing
  [target-path]
  (let
    [file-path (io/file target-path)]
    (map file-structure
         (.listFiles file-path))
  )
)

(defn tree-decorator 
  [depth]
  (if (> depth 0)
    (str (join "" (repeat (- depth 1) " ")) "\\" "_ ")
    ""
  )
)

(defn ls-shallow 
  [files]
  (join " " (map formatted-file files))
)

(defn ls-tree
  (
    [target-path files]
    (ls-tree target-path files 0)
  )

  (
    [target-path files depth]
    (do
      (join "\n" 
        (mapcat 
          (fn [file]
            (let
              [{ name :name is-directory :is-directory } file
               new-path (str target-path "/" name)
               prefix (tree-decorator depth)]
              (remove nil? 
                (concat (list (str prefix (formatted-file file)) 
                  (when is-directory 
                    (ls-tree new-path (shallow-directory-listing new-path) (inc depth)) 
                  )
                ))
              )
            )
          ) files
        )
      )
    )
  )
)

(defn list-files 
  [tree-formatted target-path]
  (let 
    [files (shallow-directory-listing target-path)]
    (if tree-formatted
      (ls-tree target-path files)
      (ls-shallow files)      
    )
  )
)

(defn -main
  [& args]
  (let 
    [{ is-tree-format :is-tree-format target-path :path } (configure args)]
    (do
      (println (list-files is-tree-format target-path))
    )
  )
)
