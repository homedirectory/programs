#lang racket
(require "helpers.rkt")
(provide read-booklist
         find-book-by-name
         )

(define BOOKS-FILE "data.csv")

(define (read-booklist)
  (file->lines BOOKS-FILE))

(define (find-book-by-name name head books)
  ;(show "Name: " name)
  (let ([regx (string-join (list "(?i:" name ")") "")])
    (filter [lambda (book)
              (regexp-match regx (car book))]
            books))
  )
