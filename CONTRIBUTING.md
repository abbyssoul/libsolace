# Contributing

Thank you so much for your interest in contributing!
This project is in active development and all contributions are welcomed.

There are multiple ways you can help. For example you can [Submit a patch](#submitting-patch) or you can [Contribute documentation](#submitting-docs). If you have encountered an issue while using the library in your project please [Report errors or bugs](#reporting-bugs) to help us fix the issue. If you feel that this library is misssing something feel free to [Request a feature/improvement](#requesting-feature) - it might be that other people can benefit from this feature.

## Submit a patch
If you want to fix an issue or add a feature - code contribution is the best way to do it. Easiest way to submit a patch is by raising a pull request (PR):

* Clone the project.
* Make any necessary changes to the source code. Please see [coding convention](docs/coding_convention.md) about code style.
* Include any [additional documentation](#contribute-documentation) the changes might be needed.
* When adding new feature it is essential to write tests that verify that your contribution works as expected.
* When fixing exising issues it is essential to update existing tests accordingly.
* All code changes are subject to automatic code quality checks run as part of CI. Code changes can not be accepted if they fail quality check so please make sure this checks pass by run `make codecheck` locally before submitting.
* Write clear, concise commit message(s) using [conventional-changelog format](https://github.com/conventional-changelog/conventional-changelog-angular/blob/master/convention.md).
* Go to https://github.com/abbyssoul/libsolace/pulls and open a new pull request with your changes.
* If your PR is connected to an open issue, add a line in your PR's description that says `Fixes: #123`, where `#123` is the number of the issue you're fixing.

### Testing
This project is commited to maintaining higest level of code quality. That is why any new feature submitted must be covered by expensive unit tests and pass code quality checks.
For fixes it is natural that existing test coverage was not sufficent to vet it. Thus it is expected that exising tests will be extended to cover issue being fixed when ever possible. This helps to avoid re-occurance of the issue as code changes over time.


## Reporting errors or bugs
If you think you run into an error or bug with the project please let us know. It is possible that other users also might experience issues. To report an issues:
* Open an Issue at https://github.com/abbyssoul/libsolace/issues
* Include *reproduction steps* that someone else can follow to recreate the bug or error on their own.
* It helps if you can specify what expected behaviour/outcome was and what was the actual result.
* Provide project and platform versions (OS name, compiler version), depending on what seems relevant. If not, please be ready to provide that information if maintainers ask for it.


## Request a feature/improvement
If the project doesn't do something you need or want it to do you can create a feature request. It is possible that there are othere users who need the same functions. To request a feature:

* Open an Issue at https://github.com/abbyssoul/libsolace/issues
* Provide as much context as you can about what you're running into.
* Please try and be clear about why existing features and alternatives would not work for you.

## Contributing documentation
Documentation is essential part of any project. To contribute documentation:

* Clone the project.
* Edit or add any relevant documentation.
* Make sure your changes are formatted correctly and consistently with the rest of the documentation.
* Re-read what you wrote, and run a spellchecker on it to make sure you didn't miss anything.
* Write clear, concise commit message(s) using [conventional-changelog format](https://github.com/conventional-changelog/conventional-changelog-angular/blob/master/convention.md). Documentation commits should use `docs(<component>): <message>`.
* Go to https://github.com/abbyssoul/libsolace/pulls and open a new pull request with your changes.
* If your PR is connected to an open issue, add a line in your PR's description that says `Fixes: #123`, where `#123` is the number of the issue you're fixing.
