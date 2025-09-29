# Contributing to LL-Connect

Thank you for your interest in contributing to LL-Connect! This document provides guidelines for contributing to the project.

## How to Contribute

### Reporting Issues
- Use the [GitHub Issues](https://github.com/Emil224411/ll-connect/issues) page
- Include detailed information about your system and the issue
- Provide steps to reproduce the problem
- Include relevant log files and error messages

### Code Contributions
- Fork the repository
- Create a feature branch: `git checkout -b feature/your-feature-name`
- Make your changes following the coding standards
- Test your changes thoroughly
- Submit a pull request with a clear description

## Development Setup

### Prerequisites
- Linux development environment
- Git
- Make
- GCC or Clang
- SDL2 development libraries

### Building from Source
```bash
git clone https://github.com/Emil224411/ll-connect.git
cd ll-connect
make
```

### Testing
- Test on multiple Linux distributions if possible
- Verify functionality with supported hardware
- Check for memory leaks and performance issues
- Ensure compatibility with different kernel versions

## Coding Standards

### C Code Style
- Follow C99 standard
- Use descriptive variable names
- Add comprehensive comments for USB protocol analysis
- Follow Linux kernel coding style for kernel modules
- Use consistent indentation (4 spaces)

### Documentation
- Update relevant documentation for new features
- Include examples and usage instructions
- Document any new protocol discoveries
- Update the README if needed

### Commit Messages
- Use descriptive commit messages
- Follow the format: "Add/Fix/Update: Brief description"
- Include more details in the commit body if needed
- Reference issues when applicable

## Areas for Contribution

### High Priority
- **Additional Device Support**: Add support for more Lian Li devices
- **Protocol Analysis**: Help reverse engineer new device protocols
- **Bug Fixes**: Fix existing issues and improve stability
- **Testing**: Test on different hardware and Linux distributions

### Medium Priority
- **UI Improvements**: Enhance the graphical interface
- **New Features**: Add new lighting effects or control options
- **Performance**: Optimize code for better performance
- **Documentation**: Improve existing documentation

### Low Priority
- **Code Refactoring**: Clean up and optimize existing code
- **Build System**: Improve Makefiles and build process
- **Packaging**: Create distribution packages
- **Translations**: Add support for multiple languages

## Device Support Guidelines

### Adding New Device Support
1. **Capture Protocol**: Use Wireshark to capture USB traffic
2. **Document Findings**: Create protocol reference document
3. **Implement Driver**: Add device-specific code
4. **Test Thoroughly**: Verify all features work correctly
5. **Update Documentation**: Add device to supported hardware list

### Protocol Analysis Process
1. Follow the [Wireshark Capture Guide](docs/wireshark-windows.md)
2. Use the [Capture Log Template](docs/templates/capture-log-template.md)
3. Document findings in `docs/protocols/`
4. Include hex dumps and command structures
5. Test against multiple firmware versions

## Pull Request Process

### Before Submitting
- [ ] Code follows project coding standards
- [ ] All tests pass
- [ ] Documentation is updated
- [ ] Commit messages are descriptive
- [ ] No merge conflicts

### Pull Request Template
```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Code refactoring
- [ ] Device support addition

## Testing
- [ ] Tested on supported hardware
- [ ] Verified no regressions
- [ ] Updated documentation

## Checklist
- [ ] Code follows project standards
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] No breaking changes
```

## Code Review Process

### Reviewers
- All pull requests require review
- Maintainers will review code quality and functionality
- Community members can also provide feedback

### Review Criteria
- Code quality and adherence to standards
- Functionality and testing
- Documentation completeness
- Performance implications
- Security considerations

## Community Guidelines

### Be Respectful
- Use welcoming and inclusive language
- Be respectful of differing viewpoints
- Focus on constructive feedback
- Help others learn and grow

### Communication
- Use clear and concise language
- Provide context for your suggestions
- Ask questions when you need clarification
- Share knowledge and help others

## Getting Help

### Resources
- [Project Documentation](docs/)
- [GitHub Discussions](https://github.com/Emil224411/ll-connect/discussions)
- [Issue Tracker](https://github.com/Emil224411/ll-connect/issues)

### Contact
- Open an issue for technical questions
- Use discussions for general questions
- Tag maintainers for urgent issues

## License

By contributing to LL-Connect, you agree that your contributions will be licensed under the same MIT License that covers the project.

## Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes
- Project documentation

Thank you for contributing to LL-Connect!
