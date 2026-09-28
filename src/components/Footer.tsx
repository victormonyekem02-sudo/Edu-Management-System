export default function Footer() {
  return (
    <footer className="border-t border-white/10 bg-slate-900/50">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-12">
        <div className="grid md:grid-cols-3 gap-8">
          {/* University info */}
          <div>
            <div className="flex items-center gap-3 mb-4">
              <div className="w-10 h-10 rounded-lg bg-gradient-to-br from-indigo-500 to-purple-600 flex items-center justify-center font-bold text-sm">
                CS
              </div>
              <div>
                <h3 className="font-bold text-white">CS4433</h3>
                <p className="text-xs text-slate-400">Software Engineering</p>
              </div>
            </div>
            <p className="text-sm text-slate-400 leading-relaxed">
              National University of Lesotho<br />
              Department of Mathematics and Computer Science<br />
              Individual Assignment — Final Project
            </p>
          </div>

          {/* Key dates */}
          <div>
            <h3 className="font-bold text-white mb-4">Key Information</h3>
            <ul className="space-y-2 text-sm text-slate-400">
              <li className="flex items-center gap-2">
                <span className="text-indigo-400">📅</span> Date: 7th November 2026
              </li>
              <li className="flex items-center gap-2">
                <span className="text-indigo-400">⏱️</span> Duration: 7 weeks
              </li>
              <li className="flex items-center gap-2">
                <span className="text-indigo-400">🎯</span> Total: 100 marks
              </li>
              <li className="flex items-center gap-2">
                <span className="text-indigo-400">📄</span> Report: 6,000–8,000 words
              </li>
              <li className="flex items-center gap-2">
                <span className="text-indigo-400">🎤</span> Viva: 15–20 minutes
              </li>
            </ul>
          </div>

          {/* Tools */}
          <div>
            <h3 className="font-bold text-white mb-4">Required Tools</h3>
            <div className="flex flex-wrap gap-2">
              {['UML', 'C++17', 'CMake', 'SQLite', 'Git', 'GoogleTest', 'Catch2', 'cppcheck', 'clang-tidy'].map((tool) => (
                <span key={tool} className="px-2.5 py-1 rounded-full bg-white/5 border border-white/10 text-xs text-slate-300">
                  {tool}
                </span>
              ))}
            </div>
          </div>
        </div>

        <div className="mt-8 pt-8 border-t border-white/10 text-center">
          <p className="text-xs text-slate-500">
            Interactive Project Dashboard for CS4433 Software Engineering Assignment • 
            Based on the official assignment brief • 
            Waterfall Model with V-Model Verification & Validation
          </p>
        </div>
      </div>
    </footer>
  );
}
