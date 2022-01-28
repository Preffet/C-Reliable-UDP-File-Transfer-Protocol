import React, {useState,useRef,useEffect} from 'react';
import TodoList from './TodoList'
import uuidv4 from 'uuid/v4'

const LOCAL_STORAGE_KEY = 'todoApp.todos'

function App() {
  /* destructure an object */
  const [todos, setTodos] = useState([])
  const todoNameRef = useRef()

  /* load the saved todos*/
  useEffect(()=>{
      const storedTodos = JSON.parse(localStorage.getItem(LOCAL_STORAGE_KEY))
      if(storedTodos)setTodos(storedTodos)
    },[])

  /* mark todos as completed */
  function toggleTodo(id){
    const newTodos=[...todos]
    const todo = newTodos.find(todo => todo.id === id)
    todo.complete = !todo.complete
    setTodos(newTodos)

  }

  /* save the todos in the local storage*/
  useEffect(()=>{
    localStorage.setItem(LOCAL_STORAGE_KEY,JSON.stringify(todos))
  },[todos])

  function handleAddTodo(e)
  {
    /* access the current element we are refferencing */
    const name = todoNameRef.current.value
    /* if an input field is empty*/
    if (name==="") return
    /*set the todos*/
    setTodos(prevTodos => {return [...prevTodos,{id:uuidv4(),name:name,complete:false}]})
    /* after the value is submitted make the field empty again*/
    todoNameRef.current.value = null
  }

  function handleClearTodos()
  {
    const newTodos = todos.filter(todo => !todo.complete)
    setTodos(newTodos)
  }

  return (
    <>
    <TodoList todos={todos} toggleTodo={toggleTodo}/>
    <input ref={todoNameRef} type='text'/>
    <button onClick={handleAddTodo}>Add todo</button>
    <button onClick={handleClearTodos}>Clear completed</button>
    <div>{todos.filter(todo => !todo.complete).length} left to do</div>
    </>
  )
}

export default App;
