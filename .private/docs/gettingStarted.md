# Working with git 

## Git basic


### Git? GitHub?

- Git – A version control system that tracks changes in your code and allows you to manage different versions of a project locally.

- GitHub – A cloud-based platform that hosts Git repositories, making it easy to collaborate, share, and back up your projects online.

### Commit?

Commit – A snapshot of your changes in the repository. Each commit represents a saved point in your project’s history.

### Branch?

Branch – A separate line of development. You can create branches to work on new features or fixes without affecting the main code.

### Commit - A recovery point for your code

A commit is represented by a node. The child of a node meaning the commit make from changed parent state (parent commit) of your code. For working together, people start from a base source (also called base version), then a person develop a feature. Until you need to combine (merge) all feature into a product.

The chart below show how we work with git:

![alt text](../imgs/gettingStarted0.png)

Sometime, We may merge some feature back to base.

### Understand your git

#### See what have changed

```
git status
```
Result:
![alt text](../imgs/gettingStarted1.png)

Description:

- `On branch base` : You’re currently on the `base` branch.
- `Your branch is ahead of 'origin/base' by 2 commits.` : You have two local commits that haven’t been pushed to GitHub yet.
- `Changes not staged for commit:` : These are tracked files that have been modified but not added to the next commit.
- `Untracked files:` : These are new files not yet tracked by Git. You may have just created or added them.

#### Commit what's changed

Add specific files to the next commit:

```
git add <files that you want to commit>
```

Add all changed and untracked files:

```
git add -Av 
```

Then create a commit with a short message:

```
git commit -m <Short mesage>
```

Or open the default editor to write a detailed commit message:

```
git commit 
```

Result:

![alt text](../imgs/gettingStarted2.png)

### Conflicts - Unavoidable
 
The part above shows an ideal workflow. In real life, it’s much more complicated. If a node (a state or commit) has two child nodes with similar changes in different branches, then when you merge those branches, a conflict is very likely to occur. Simply put, if you merge two commits that modify the same lines or positions in the code, a conflict will occur.

There are three ways to solve the config, but in that section, i will talk about one of them. 

**Rebase**
