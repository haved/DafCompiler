;; The mode for the daf programming language
;; So far just syntax highlighting

(setq daf-keywords '("pub" "prot" "let" "def" "mut" "typedef" "namedef" "with" "as" "linkfile" "move" "uncrt" "certain" "class" "trait" "enum" "virt" "ctor" "dtor"))
(setq daf-control '("if" "else" "for" "while" "break" "continue" "retry" "return"))
(setq daf-literals '("null" "this" "true" "false"))
(setq daf-types '("This" "u8" "i8" "u16" "i16" "u32" "i32" "u64" "i64" "f32" "f64" "char" "usize" "isize" "bool"))

(setq daf-keywords-regexp (regexp-opt daf-keywords 'words))
(setq daf-control-regexp (regexp-opt daf-control 'words))
(setq daf-literals-regexp (regexp-opt daf-literals 'words))
(setq daf-types-regexp (regexp-opt daf-types 'words))

(setq daf-font-lock-keywords
	  `(
		(,"//.*$" . font-lock-comment-face)
		;; TODO: Add multi line comments
		(,daf-types-regexp . font-lock-type-face)
        (,daf-literals-regexp . font-lock-constant-face)
        (,daf-control-regexp . font-lock-builtin-face)
        (,daf-keywords-regexp . font-lock-keyword-face)
				))

;;;###autoload
(define-derived-mode daf-mode fundamental-mode "daf mode"
	"Major mode for editing daf files"

	;;syntax highlighting
	(setq font-lock-defaults '((daf-font-lock-keywords))))

(setq daf-keywords nil)
(setq daf-control nil)
(setq daf-literals nil)
(setq daf-types nil)

(setq daf-keywords-regexp nil)
(setq daf-control-regexp nil)
(setq daf-literals-regexp nil)
(setq daf-types-regexp nil)

(provide 'daf-mode)
