;;; tela-mode.el --- Tela mode, and its idiosyncratic commands.

;; Copyright (C) 2001 Mikael Hedin (micce@debian.org)

;; Keywords: tela, languages

;; Tela-mode is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; Tela-mode is distributed in the hope that it will be useful, but
;; WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;; General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with TEla-mode; see the file COPYING.  If not, write to the Free
;; Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
;; 02111-1307, USA.

;;; Code:

;;; Initialization
;; To use tela-mode for all t-files and c++-modes for ct-files, put
;; tela.el somwhere in the load-path (or adjust the load-path) and
;; then insert the following lines (without the leading `;') into your
;; startup file (e.g. `.emacs' or `.xemacs/init.el').
;; 
;(autoload 'tela-mode "tela" "TeLa mode" t)
;(setq auto-mode-alist (cons '("\\.t\\'" . tela-mode) auto-mode-alist))
;(setq auto-mode-alist (cons '("\\.ct\\'" . c++-mode) auto-mode-alist))


(require 'font-lock)
(require 'easymenu)

;;; Customizable part ==================================================
(defgroup tela nil
  "Support for tela files."
  :group 'languages
  :prefix "tela-")

(defcustom tela-indent-level 2
  "The amount to indent a line relative to the surrounding."
  :type 'integer
  :group 'tela
  )


;;; Variables ==========================================================
(defvar tela-identifier-regexp
  "\\<[a-zA-Z$äÄöÖåÅ][a-zA-Z0-9_$äÄöÖåÅ]*\\>"
  "Regexp that match valid tela identifier, from tela source file d.l")

(defvar tela-function-regexp
  (concat "^\\s-*function\\s-+" 
	  "\\(\\(" tela-identifier-regexp "\\|"
	  "\\[[^]]*\\]" "\\)"		;match anything inside brackets, not
					;quite correct but...  
	  "\\s-*=\\s-*" "\\)?"
	  "\\(" tela-identifier-regexp "\\)"
	  )
  "Regexp that matches a tela function declaration, from tela source
d.y.  Paren 2 is the output argument, and paren 3 is the function
name.")

(defvar tela-eol-regexp "\\s-*\\(//.*\\)?$") 

(defvar tela-font-lock-keywords
  (list
   (list (concat "\\<\\(" (mapconcat 'identity 
				     ;;Keywords from d.l
				     '("if" "else" "while" "for"
				       "foreach" "break" "continue"
				       "return" "repeat" "until" "goto"
				       "label" "function" "package"
				       "local" "global" "call" "disp"
				       "mod")
				     "\\|") 
		 "\\)\\>") 
	 1 font-lock-keyword-face))
  "Test font-lock stuff")

;;Make use of the keywords in font-locking
(put 'tela-mode 'font-lock-defaults 'tela-font-lock-keywords)

;;imenu is used to construct a menu of functions defined in the
;;current buffer. 
(defvar tela-imenu-generic-exp
  (list
   (list nil tela-function-regexp 3)	; match function def
   ))


;;; Utilities ==========================================================

(defun tela-current-defun ()
  "Name of the function definition point is in, or the closest previous
function.  Return nil if no function name found." 
  (save-excursion (end-of-line)
		  (and (re-search-backward tela-function-regexp nil t)
		       (match-string 3))))

(defun tela-show-syntax ()
  "Show what we think is the syntax and indentation appropriate for
this line. "
  (interactive)
  (message "%s:" (tela-calculate-indentation)))

(defun tela-find-previous-line ()
  (if (= -1 (forward-line -1)) nil
    (if (or (tela-ltype-comment)
	    (tela-ltype-empty))
	(tela-find-previous-line)
      t)))

(defun tela-previous-line ()
  "Move up to the previous line of code. Return nil if not found,
i.e. already on the first line." 
  (interactive)
  (let ((old-point (point)))
    (if (tela-find-previous-line) t (goto-char old-point) nil)))


;;; Line types =========================================================

(defun tela-ltype-block-close () 
  "Return t if current line starts with a statement closing brace '}'."
  (save-excursion
    (beginning-of-line)
    (looking-at "^\\s-*}")))

(defun tela-ltype-block-open () 
  "Return t if current line ends with a statement opening brace '{'."
  (save-excursion
    (beginning-of-line)
    (looking-at ".*{\\s-*$")))

(defun tela-ltype-block-first () 
  "Return t if this is the first line of code in a block (delimited by
'{' and '}')."
  (save-excursion (and (tela-previous-line)
		       (tela-ltype-block-open)
		       (not (tela-ltype-package)))))

(defun tela-ltype-open-clause ()
  "Return t if this line opens a construct that should have a single
line following."
  (save-excursion
    (beginning-of-line)
    (looking-at (concat 
		 "\\s-*\\(if\\|while\\|for\\)\\s-*(.*)" 
		 tela-eol-regexp))))

(defun tela-ltype-stmt-cont () 
  "Return t of this line is a continuation of the statment on the
prevoius line."
  (save-excursion (and (tela-previous-line)
		       (tela-ltype-open-clause))))

(defun tela-ltype-repeat ()
  "Return t if this line starts a repeat statement with lines to
follow."
  (save-excursion (beginning-of-line)
		  (looking-at (concat 
			       "\\s-*repeat"
			       tela-eol-regexp))))
  
(defun tela-ltype-repeat-first () 
  "Return t if this is the first line of a repeat-until block."
  (save-excursion (and (tela-previous-line)
		       (tela-ltype-repeat))))

(defun tela-ltype-until () 
  "Retutn t if the line is the 'until' keyword ending of a repeat-until block."
  (save-excursion (beginning-of-line)
		  (looking-at "^\\s-*until")))

(defun tela-ltype-stmt ()
  "Return t if the line is a complete statement."
  (save-excursion (beginning-of-line)
		  (looking-at (concat ".*;" tela-eol-regexp))))

(defun tela-ltype-empty ()		; blank line
  "Return t if current line is empty."
  (save-excursion (beginning-of-line)
		  (looking-at "^\\s-*$")))

(defun tela-ltype-comment ()		; comment line
  "Return t if current line is a tela comment line ONLY '//' FOR NOW!!!"
  (save-excursion (beginning-of-line)
		  (looking-at "[ \t]*//.*$")))

(defun tela-ltype-function-definition ()
  "Return t if the current line is a function definition."
  (save-excursion (beginning-of-line)
		  (looking-at tela-function-regexp)))

(defun tela-ltype-package ()
  "Return t if the current line is a package definition opening."
  (save-excursion (beginning-of-line) 
		  (looking-at "\\s-*package")))


;;; Indent functions ===================================================
(defun tela-matching-sexp-indent ()
  "Return the indentation of the beginning of the sexp we are currently inside."
  (let* ((close (save-excursion
		  (beginning-of-line)
		  (re-search-forward "^\\s-*}")))
	 (open (scan-sexps close -1)))
    (save-excursion (goto-char open)
		    (current-indentation))))

(defun tela-previous-indent ()
  "Get the indentation of the previous code line.  Return 0 if there
is no previous line of code."
  (save-excursion 
    (if (tela-previous-line)
	(current-indentation)
      0)))

(defun tela-indent-line ()
  "Indent a line in `tela-mode'."
  (interactive)
  (let ((i (tela-calc-indent)))
    (save-excursion
      (back-to-indentation)
      (if (= i (current-column))
	  nil
	(delete-horizontal-space)
	(indent-to i)))
    (if (<= (current-column) i) (move-to-column i))
    ))

(defun tela-indent-sexp ()
  "Indent the sexp just after point."
  (interactive)
  (save-excursion 
    (let* ((start (save-restriction (narrow-to-region (point-min)
						      (point-at-eol))
				    (1- (scan-lists (point) 1 -1))))
	   (end (progn (goto-char start) (forward-sexp) (point))))
      ;;(message "start: %s; end: %s" start end)
      (indent-region start end nil))))

(defun tela-indent-defun ()
  "Indent the current function definition."
  (interactive)
  (save-excursion (beginning-of-defun) (tela-indent-sexp)))


(defun tela-calc-indent ()
  "Return the appropriate indentation for this line as an integer."
  (nth 1 (tela-calculate-indentation)))

(defun tela-calculate-indentation ()
  "Calculate the indentation of the current line. Return a list of
descriptions for this line.  Returns a list (TYPE DEPTHNUMBER) where
TYPE describes the line type and DEPTHNUMBER is how many characters to
indent this line." 
  (cond
   ;; COMMENTS---Skip them, and treat them as code lines.
   ;;  ((tela-ltype-comment)
   ;;   (list 'comment 0))
   ;; PACKAGE LINE
   ((tela-ltype-package)
    (list 'package 0))
   ;; FUNCTION DEFINITION
   ((tela-ltype-function-definition)
    (list 'fun-def 0))
   ;; BLOCK CLOSING
   ((tela-ltype-block-close)
    (list 'block-close (tela-matching-sexp-indent)))
   ;; FIRST LINE IN BLOCK
   ((tela-ltype-block-first)
    (list 'block-first-line  (+ tela-indent-level
				(tela-previous-indent))))
   ;; STMT CONTINUATION
   ((tela-ltype-stmt-cont)
    (list 'stmt-cont (+ tela-indent-level
			(tela-previous-indent))))
   ;; REPEAT BLOCK LINES
   ((tela-ltype-repeat-first)
    (list 'repeat-block-first (+ tela-indent-level
				 (tela-previous-indent))))
   ;; UNTIIL LINE
   ((tela-ltype-until)
    (list 'until (save-excursion (while (not (tela-ltype-repeat))
				   (tela-previous-line))
				 (current-indentation))))
   ;; OTHER LINES--ordinary code and comments
   (t
    (list 'line (if (save-excursion 
		      (and (tela-previous-line)
			   (and (eq (car (tela-calculate-indentation))
				    'stmt-cont) 
				(setq tmp (tela-previous-indent)))))
		    tmp
		  (tela-previous-indent))))))


;;;The main function ===================================================

(defvar tela-mode-abbrev-table nil
  "Abbrev table used in `tela-mode' buffers.")

(define-abbrev-table 'tela-mode-abbrev-table nil)

(defvar tela-mode-map nil
  "Keymap used in `tela-mode' buffers.")

;;Set only if not already defined by the user
(if tela-mode-map nil
  (setq tela-mode-map (make-sparse-keymap))
  (define-key tela-mode-map [(meta control q)] 'tela-indent-sexp)
  (define-key tela-mode-map [(control c) (control c)] 'tela-show-syntax)
  (define-key tela-mode-map [(control c) (control s)] 'tela-show-syntax)
  (define-key tela-mode-map [(control c) (control q)] 'tela-indent-defun)
  )

(defvar tela-mode-syntax-table nil
  "Syntax table used in `tela-mode' buffers.")

(if tela-mode-syntax-table nil
  (setq tela-mode-syntax-table (make-syntax-table))
  (modify-syntax-entry ?_  "_"     tela-mode-syntax-table)
  (modify-syntax-entry ?\\ "\\"    tela-mode-syntax-table)
  (modify-syntax-entry ?+  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?-  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?=  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?%  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?<  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?>  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?&  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?|  "."     tela-mode-syntax-table)
  (modify-syntax-entry ?\' "\\"    tela-mode-syntax-table)
  (modify-syntax-entry ?/ ". 1456" tela-mode-syntax-table)
  (modify-syntax-entry ?* ". 23" tela-mode-syntax-table)
  (modify-syntax-entry ?\n "> b" tela-mode-syntax-table))

(easy-menu-define 
 tela-menu tela-mode-map "Tela menu" 
 '("Tela"
   ["show suntax" tela-show-syntax t]
   ))

(defun tela-mode ()
  "Major mode for editing tela-files.
\\{tela-mode-map}
Experimental verion."
  (interactive)
  (kill-all-local-variables)
  (make-local-variable 'add-log-current-defun-function)
  (make-local-variable 'comment-column)
  (make-local-variable 'comment-end)
  (make-local-variable 'comment-indent-function)
  (make-local-variable 'comment-start)
  (make-local-variable 'comment-start-skip)
  (make-local-variable 'font-lock-defaults)
  (make-local-variable 'indent-line-function)
  (make-local-variable 'indent-region-function)
  (make-local-variable 'paragraph-separate)
  (make-local-variable 'paragraph-start)

  (setq 
   add-log-current-defun-function 'tela-current-defun
   ;;Make auto-filled lines start with `comment-start'
   comment-multi-line nil		
   comment-end ""
   comment-start "//"
   comment-start-skip "//+\\s-*"
   ;;	comment-indent-function 'tela-comment-indent
   imenu-generic-expression tela-imenu-generic-exp
   indent-line-function 'tela-indent-line
   ;;	indent-region-function 'tela-indent-region
   local-abbrev-table tela-mode-abbrev-table
   mode-name "Tela"
   paragraph-separate "^\\s-*$" 
   paragraph-start paragraph-separate
   major-mode 'tela-mode
   )
  (use-local-map tela-mode-map)
  (set-syntax-table tela-mode-syntax-table)
  (easy-menu-add tela-menu)
  (imenu-add-to-menubar "Tela-functions")
  (run-hooks 'tela-mode-hook))
