You can run `doxygen` from this subdirectory of your local repository
to generate a local copy of the documentation. The results will appear
in an `html` subdirectory, and the main page is `html/index.html`.

Also, a "GitHub action" is set up to automatically run `doxygen` from
this subdirectory every time there is a commit to master, and the
results are copied to the `gh-pages` branch, so they appear
online at 
[https://hiddensymmetries.github.io/mango](https://hiddensymmetries.github.io/mango).
The GitHub action is configured in `/.github/workflows/docs.yml`.