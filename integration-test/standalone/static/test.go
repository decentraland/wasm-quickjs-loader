package main

import (
    "fmt"; "log"; "net/http"
)

func cors(fs http.Handler) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
			// do your cors stuff
			// return if you do not want the FileServer handle a specific request

			w.Header().Set("Cross-Origin-Opener-Policy", "same-origin")
			w.Header().Set("Cross-Origin-Embedder-Policy", "require-corp")

			fs.ServeHTTP(w, r)
	}
}

func main() {
    fmt.Println("Serving files in the current directory on port 8080")
		http.Handle("/", cors(http.FileServer(http.Dir("."))))
    err := http.ListenAndServe(":8080", nil)
    if err != nil {
        log.Fatal("ListenAndServe: ", err)
    }
}