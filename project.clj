(defproject ls-clone-cloj "0.1.0-SNAPSHOT"
  :description "A common UNIX ls utility clone written for fun and learning Clojure purposes"
  :url "http://github.com/Killavus/ls-clone-cloj"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [org.clojure/tools.cli "0.3.3"]]
  :main ^:skip-aot ls-clone-cloj.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}})
