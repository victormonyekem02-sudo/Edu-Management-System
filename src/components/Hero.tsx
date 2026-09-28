export default function Hero() {
  return (
    <section id="overview" className="relative overflow-hidden">
      {/* Background decoration */}
      <div className="absolute inset-0 overflow-hidden pointer-events-none">
        <div className="absolute -top-40 -right-40 w-80 h-80 bg-indigo-500/10 rounded-full blur-3xl" />
        <div className="absolute -bottom-40 -left-40 w-80 h-80 bg-purple-500/10 rounded-full blur-3xl" />
      </div>

      <div className="relative max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-16 lg:py-24">
        <div className="text-center mb-12">
          <div className="inline-flex items-center gap-2 px-4 py-2 rounded-full bg-indigo-500/10 border border-indigo-500/20 text-indigo-300 text-sm mb-6">
            <span className="w-2 h-2 rounded-full bg-green-400 animate-pulse" />
            National University of Lesotho • Department of Mathematics & Computer Science
          </div>
          <h1 className="text-4xl sm:text-5xl lg:text-6xl font-bold bg-gradient-to-r from-white via-indigo-200 to-purple-200 bg-clip-text text-transparent mb-4">
            CS4433 Software Engineering
          </h1>
          <h2 className="text-xl sm:text-2xl text-slate-300 mb-2">Individual Assignment — Final Project</h2>
          <p className="text-slate-400 max-w-2xl mx-auto">
            Moderated Learning Management System — Full Software Engineering Life Cycle
          </p>
        </div>

        {/* Key info cards */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-12">
          {[
            { label: 'Duration', value: '7 Weeks', icon: '⏱️', color: 'from-blue-500/20 to-blue-600/20' },
            { label: 'Total Marks', value: '100', icon: '🎯', color: 'from-green-500/20 to-green-600/20' },
            { label: 'Model', value: 'Waterfall + V', icon: '🔄', color: 'from-purple-500/20 to-purple-600/20' },
            { label: 'Language', value: 'C++17', icon: '💻', color: 'from-orange-500/20 to-orange-600/20' },
          ].map((card) => (
            <div
              key={card.label}
              className={`bg-gradient-to-br ${card.color} backdrop-blur-sm rounded-xl p-4 border border-white/10 text-center`}
            >
              <div className="text-2xl mb-1">{card.icon}</div>
              <div className="text-lg font-bold text-white">{card.value}</div>
              <div className="text-xs text-slate-400">{card.label}</div>
            </div>
          ))}
        </div>

        {/* Case study summary */}
        <div className="bg-white/5 backdrop-blur-sm rounded-2xl border border-white/10 p-6 lg:p-8">
          <h3 className="text-xl font-bold text-white mb-4 flex items-center gap-2">
            <span className="text-2xl">📖</span> Case Study: Moderated Learning Management System
          </h3>
          <p className="text-slate-300 mb-6 leading-relaxed">
            A secondary school wants a Learning Management System (LMS) to manage courses, assignments and assessment. 
            The school has four types of user: <strong className="text-indigo-300">students</strong>, <strong className="text-purple-300">lecturers</strong>, 
            <strong className="text-amber-300"> Heads of Department (HoDs)</strong> and a <strong className="text-green-300">system administrator</strong>. 
            Every user logs in with a username and password and can only perform the actions allowed by their role.
          </p>

          <div className="grid md:grid-cols-2 lg:grid-cols-4 gap-4">
            {[
              {
                role: 'Administrator',
                color: 'border-green-500/30 bg-green-500/5',
                icon: '🛡️',
                tasks: ['Manages user accounts', 'Manages departments & courses', 'Assigns lecturers to courses', 'Enrols students'],
              },
              {
                role: 'Lecturer',
                color: 'border-purple-500/30 bg-purple-500/5',
                icon: '👨‍🏫',
                tasks: ['Creates assignments', 'Sets deadlines & max marks', 'Grants extensions', 'Marks & writes feedback'],
              },
              {
                role: 'Head of Department',
                color: 'border-amber-500/30 bg-amber-500/5',
                icon: '🎓',
                tasks: ['Reviews & moderates marks', 'Approves marks for release', 'Oversees all dept. courses', 'Releases marks to students'],
              },
              {
                role: 'Student',
                color: 'border-blue-500/30 bg-blue-500/5',
                icon: '📚',
                tasks: ['Views courses & assignments', 'Submits work', 'Resubmits before deadline', 'Views released marks & feedback'],
              },
            ].map((item) => (
              <div key={item.role} className={`rounded-xl border p-4 ${item.color}`}>
                <div className="flex items-center gap-2 mb-3">
                  <span className="text-xl">{item.icon}</span>
                  <h4 className="font-semibold text-white text-sm">{item.role}</h4>
                </div>
                <ul className="space-y-1.5">
                  {item.tasks.map((task) => (
                    <li key={task} className="text-xs text-slate-300 flex items-start gap-1.5">
                      <span className="text-indigo-400 mt-0.5">•</span>
                      {task}
                    </li>
                  ))}
                </ul>
              </div>
            ))}
          </div>

          <div className="mt-6 p-4 rounded-lg bg-red-500/5 border border-red-500/20">
            <h4 className="text-sm font-semibold text-red-300 mb-2">⚠️ Out of Scope</h4>
            <p className="text-xs text-slate-400">
              Online payment, video conferencing, automated grading, plagiarism detection, and web or mobile interfaces. 
              The system will be a C++ application (console or simple desktop GUI) running on a single school computer or local network.
            </p>
          </div>
        </div>

        {/* Tools */}
        <div className="mt-8 flex flex-wrap justify-center gap-3">
          {['UML Tool', 'C++17', 'CMake', 'SQLite', 'Git', 'GoogleTest/Catch2', 'cppcheck/clang-tidy'].map((tool) => (
            <span
              key={tool}
              className="px-3 py-1.5 rounded-full bg-white/5 border border-white/10 text-xs text-slate-300"
            >
              {tool}
            </span>
          ))}
        </div>
      </div>
    </section>
  );
}
